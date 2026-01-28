#include "projection.h"

#include "../geometry/pose.h"

namespace colmap {

double CalculateSquaredReprojectionError(const Eigen::Vector2d& point2D,
                                         const Eigen::Vector3d& point3D,
                                         const Rigid3d& cam_from_world,
                                         const Camera& camera) {
  const Eigen::Vector3d point3D_in_cam = cam_from_world * point3D;
  const std::optional<Eigen::Vector2d> proj_point2D =
      camera.ImgFromCam(point3D_in_cam);
  if (!proj_point2D) {
    return std::numeric_limits<double>::max();
  }
  return (*proj_point2D - point2D).squaredNorm();
}

double CalculateSquaredReprojectionError(
    const Eigen::Vector2d& point2D,
    const Eigen::Vector3d& point3D,
    const Eigen::Matrix3x4d& cam_from_world,
    const Camera& camera) {
  const Eigen::Vector3d point3D_in_cam = cam_from_world * point3D.homogeneous();
  const std::optional<Eigen::Vector2d> proj_point2D =
      camera.ImgFromCam(point3D_in_cam);
  if (!proj_point2D) {
    return std::numeric_limits<double>::max();
  }
  return (*proj_point2D - point2D).squaredNorm();
}

double CalculateAngularReprojectionError(const Eigen::Vector2d& point2D,
                                         const Eigen::Vector3d& point3D,
                                         const Rigid3d& cam_from_world,
                                         const Camera& camera) {
  const std::optional<Eigen::Vector2d> cam_point = camera.CamFromImg(point2D);
  if (!cam_point) {
    return EIGEN_PI;
  }
  return CalculateAngularReprojectionError(
      cam_point->homogeneous().normalized(), point3D, cam_from_world);
}

double CalculateAngularReprojectionError(
    const Eigen::Vector2d& point2D,
    const Eigen::Vector3d& point3D,
    const Eigen::Matrix3x4d& cam_from_world,
    const Camera& camera) {
  const std::optional<Eigen::Vector2d> cam_point = camera.CamFromImg(point2D);
  if (!cam_point) {
    return EIGEN_PI;
  }
  return CalculateAngularReprojectionError(
      cam_point->homogeneous().normalized(), point3D, cam_from_world);
}

double CalculateAngularReprojectionError(const Eigen::Vector3d& cam_ray,
                                         const Eigen::Vector3d& point3D,
                                         const Rigid3d& cam_from_world) {
  const Eigen::Vector3d point3D_in_cam = cam_from_world * point3D;
  return std::acos(cam_ray.transpose() * point3D_in_cam.normalized());
}

double CalculateAngularReprojectionError(
    const Eigen::Vector3d& cam_ray,
    const Eigen::Vector3d& point3D,
    const Eigen::Matrix3x4d& cam_from_world) {
  const Eigen::Vector3d point3D_in_cam = cam_from_world * point3D.homogeneous();
  return std::acos(cam_ray.transpose() * point3D_in_cam.normalized());
}

bool HasPointPositiveDepth(const Eigen::Matrix3x4d& cam_from_world,
                           const Eigen::Vector3d& point3D) {
  return cam_from_world.row(2).dot(point3D.homogeneous()) >=
         std::numeric_limits<double>::epsilon();
}

}  // namespace colmap
