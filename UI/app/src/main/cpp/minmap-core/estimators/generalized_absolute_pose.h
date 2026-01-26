#pragma once

#include "../geometry/rigid3.h"
#include "../util/types.h"

#include <vector>

#include <PoseLib/alignment.h>

//#define EIGEN_DONT_VECTORIZE
//#define EIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <Eigen/Dense>

namespace colmap {

// Solver for the Generalized P3P problem.
class GP3PEstimator {
 public:
  // The generalized image observations, which is composed of the relative pose
  // of a camera in the generalized camera and a ray in the camera frame.
  struct X_t {
    Rigid3d cam_from_rig;
    Eigen::Vector3d ray_in_cam;
  };

  // The observed 3D feature points in the world frame.
  typedef Eigen::Vector3d Y_t;
  // The estimated rig_from_world pose of the generalized camera.
  typedef Rigid3d M_t;

  // The minimum number of samples needed to estimate a model.
  static const int kMinNumSamples = 3;

  // Whether to compute the cosine similarity or the reprojection error.
  // [WARNING] The reprojection error being in normalized coordinates,
  // the unique error threshold of RANSAC corresponds to different pixel values
  // in the different cameras of the rig if they have different intrinsics.
  enum class ResidualType {
    CosineDistance,
    ReprojectionError,
  };

  explicit GP3PEstimator(
      ResidualType residual_type = ResidualType::CosineDistance);

  // Estimate the most probable solution of the GP3P problem from a set of
  // three 2D-3D point correspondences.
  static void Estimate(const std::vector<X_t>& points2D,
                       const std::vector<Y_t>& points3D,
                       std::vector<M_t>* models);

  // Calculate the squared cosine distance error between the rays given a set of
  // 2D-3D point correspondences and the rig pose of the generalized camera.
  void Residuals(const std::vector<X_t>& points2D,
                 const std::vector<Y_t>& points3D,
                 const M_t& rig_from_world,
                 std::vector<double>* residuals) const;

 private:
  const ResidualType residual_type_;
};

}  // namespace colmap
