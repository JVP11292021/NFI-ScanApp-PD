#include "triangulation.h"

#include "../geometry/essential_matrix.h"
#include "../geometry/pose.h"

#include <PoseLib/alignment.h>
#include <Eigen/Dense>

namespace colmap {

bool TriangulatePoint(const Eigen::Matrix3x4d& cam1_from_world,
                      const Eigen::Matrix3x4d& cam2_from_world,
                      const Eigen::Vector2d& cam_point1,
                      const Eigen::Vector2d& cam_point2,
                      Eigen::Vector3d* xyz) {
  THROW_CHECK_NOTNULL(xyz);

  Eigen::Matrix4d A;
  A.row(0) = cam_point1(0) * cam1_from_world.row(2) - cam1_from_world.row(0);
  A.row(1) = cam_point1(1) * cam1_from_world.row(2) - cam1_from_world.row(1);
  A.row(2) = cam_point2(0) * cam2_from_world.row(2) - cam2_from_world.row(0);
  A.row(3) = cam_point2(1) * cam2_from_world.row(2) - cam2_from_world.row(1);

  const Eigen::JacobiSVD<Eigen::Matrix4d> svd(A, Eigen::ComputeFullV);
#if EIGEN_VERSION_AT_LEAST(3, 4, 0)
  if (svd.info() != Eigen::Success) {
    return false;
  }
#endif

  if (svd.matrixV()(3, 3) == 0) {
    return false;
  }

  *xyz = svd.matrixV().col(3).hnormalized();
  return true;
}

bool TriangulateMidPoint(const Rigid3d& cam2_from_cam1,
                         const Eigen::Vector3d& cam_ray1,
                         const Eigen::Vector3d& cam_ray2,
                         Eigen::Vector3d* point3D_in_cam1) {
  const Eigen::Quaterniond cam1_from_cam2_rotation =
      cam2_from_cam1.rotation.inverse();
  const Eigen::Vector3d cam_ray2_in_cam1 = cam1_from_cam2_rotation * cam_ray2;
  const Eigen::Vector3d cam2_in_cam1 =
      cam1_from_cam2_rotation * -cam2_from_cam1.translation;

  Eigen::Matrix3d A;
  A << cam_ray1(0), -cam_ray2_in_cam1(0), -cam2_in_cam1(0), cam_ray1(1),
      -cam_ray2_in_cam1(1), -cam2_in_cam1(1), cam_ray1(2), -cam_ray2_in_cam1(2),
      -cam2_in_cam1(2);

  const Eigen::JacobiSVD<Eigen::Matrix3d> svd(A, Eigen::ComputeFullV);
#if EIGEN_VERSION_AT_LEAST(3, 4, 0)
  if (svd.info() != Eigen::Success) {
    return false;
  }
#endif

  if (svd.matrixV()(2, 2) == 0) {
    return false;
  }

  const Eigen::Vector2d lambda = svd.matrixV().col(2).hnormalized();

  // Check if point is behind cameras.
  if (lambda(0) <= std::numeric_limits<double>::epsilon() ||
      lambda(1) <= std::numeric_limits<double>::epsilon()) {
    return false;
  }

  *point3D_in_cam1 = 0.5 * (lambda(0) * cam_ray1 + cam2_in_cam1 +
                            lambda(1) * cam_ray2_in_cam1);

  return true;
}

bool TriangulateMultiViewPoint(
    const span<const Eigen::Matrix3x4d>& cams_from_world,
    const span<const Eigen::Vector2d>& cam_points,
    Eigen::Vector3d* xyz) {
  THROW_CHECK_EQ(cams_from_world.size(), cam_points.size());
  THROW_CHECK_NOTNULL(xyz);

  Eigen::Matrix4d A = Eigen::Matrix4d::Zero();
  for (size_t i = 0; i < cam_points.size(); i++) {
    const Eigen::Vector3d point = cam_points[i].homogeneous().normalized();
    const Eigen::Matrix3x4d term =
        cams_from_world[i] - point * point.transpose() * cams_from_world[i];
    A += term.transpose() * term;
  }

  const Eigen::SelfAdjointEigenSolver<Eigen::Matrix4d> eigen_solver(A);
  if (eigen_solver.info() != Eigen::Success ||
      eigen_solver.eigenvectors()(3, 0) == 0) {
    return false;
  }

  *xyz = eigen_solver.eigenvectors().col(0).hnormalized();
  return true;
}

bool TriangulateOptimalPoint(const Eigen::Matrix3x4d& cam1_from_world_mat,
                             const Eigen::Matrix3x4d& cam2_from_world_mat,
                             const Eigen::Vector2d& cam_point1,
                             const Eigen::Vector2d& cam_point2,
                             Eigen::Vector3d* xyz) {
  const Rigid3d cam1_from_world(
      Eigen::Quaterniond(cam1_from_world_mat.leftCols<3>()),
      cam1_from_world_mat.col(3));
  const Rigid3d cam2_from_world(
      Eigen::Quaterniond(cam2_from_world_mat.leftCols<3>()),
      cam2_from_world_mat.col(3));
  const Rigid3d cam2_from_cam1 = cam2_from_world * Inverse(cam1_from_world);
  const Eigen::Matrix3d E = EssentialMatrixFromPose(cam2_from_cam1);

  Eigen::Vector2d optimal_point1;
  Eigen::Vector2d optimal_point2;
  FindOptimalImageObservations(
      E, cam_point1, cam_point2, &optimal_point1, &optimal_point2);

  return TriangulatePoint(cam1_from_world_mat,
                          cam2_from_world_mat,
                          optimal_point1,
                          optimal_point2,
                          xyz);
}

namespace {

inline double CalculateTriangulationAngleWithKnownBaseline(
    double baseline_length_squared,
    const Eigen::Vector3d& proj_center1,
    const Eigen::Vector3d& proj_center2,
    const Eigen::Vector3d& point3D) {
  const double ray_length_squared1 = (point3D - proj_center1).squaredNorm();
  const double ray_length_squared2 = (point3D - proj_center2).squaredNorm();

  // Using "law of cosines" to compute the enclosing angle between rays.
  const double denominator =
      2.0 * std::sqrt(ray_length_squared1 * ray_length_squared2);
  if (denominator == 0.0) {
    return 0.0;
  }
  const double nominator =
      ray_length_squared1 + ray_length_squared2 - baseline_length_squared;
  const double angle =
      std::acos(std::clamp(nominator / denominator, -1.0, 1.0));

  // Triangulation is unstable for acute angles (far away points) and
  // obtuse angles (close points), so always compute the minimum angle
  // between the two intersecting rays.
  return std::min(angle, M_PI - angle);
}

}  // namespace

double CalculateTriangulationAngle(const Eigen::Vector3d& proj_center1,
                                   const Eigen::Vector3d& proj_center2,
                                   const Eigen::Vector3d& point3D) {
  const double baseline_length_squared =
      (proj_center1 - proj_center2).squaredNorm();
  return CalculateTriangulationAngleWithKnownBaseline(
      baseline_length_squared, proj_center1, proj_center2, point3D);
}

std::vector<double> CalculateTriangulationAngles(
    const Eigen::Vector3d& proj_center1,
    const Eigen::Vector3d& proj_center2,
    const std::vector<Eigen::Vector3d>& points3D) {
  const double baseline_length_squared =
      (proj_center1 - proj_center2).squaredNorm();
  std::vector<double> angles(points3D.size());
  for (size_t i = 0; i < points3D.size(); ++i) {
    angles[i] = CalculateTriangulationAngleWithKnownBaseline(
        baseline_length_squared, proj_center1, proj_center2, points3D[i]);
  }
  return angles;
}

}  // namespace colmap
