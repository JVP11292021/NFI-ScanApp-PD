#pragma once

#include "../geometry/rigid3.h"
#include "../geometry/sim3.h"
#include "../util/types.h"

#include <vector>

#include <PoseLib/alignment.h>
#include <Eigen/Core>

namespace colmap {

// Compute the closes rotation matrix with the closest Frobenius norm by setting
// the singular values of the given matrix to 1.
Eigen::Matrix3d ComputeClosestRotationMatrix(const Eigen::Matrix3d& matrix);

// Convert 3D rotation matrix to Euler angles.
//
// The convention `R = Rx * Ry * Rz` is used,
// using a right-handed coordinate system.
//
// @param R              3x3 rotation matrix.
// @param rx, ry, rz     Euler angles in radians.
void RotationMatrixToEulerAngles(const Eigen::Matrix3d& R,
                                 double* rx,
                                 double* ry,
                                 double* rz);

// Convert Euler angles to 3D rotation matrix.
//
// The convention `R = Rz * Ry * Rx` is used,
// using a right-handed coordinate system.
//
// @param rx, ry, rz     Euler angles in radians.
//
// @return               3x3 rotation matrix.
Eigen::Matrix3d EulerAnglesToRotationMatrix(double rx, double ry, double rz);

// Compute the weighted average of multiple Quaternions according to:
//
//    Markley, F. Landis, et al. "Averaging quaternions."
//    Journal of Guidance, Control, and Dynamics 30.4 (2007): 1193-1197.
//
// @param quats         The Quaternions to be averaged.
// @param weights       Non-negative weights.
//
// @return              The average Quaternion.
Eigen::Quaterniond AverageQuaternions(
    const std::vector<Eigen::Quaterniond>& quats,
    const std::vector<double>& weights);

// Linearly interpolate camera pose.
Rigid3d InterpolateCameraPoses(const Rigid3d& cam1_from_world,
                               const Rigid3d& cam2_from_world,
                               double t);

// Perform cheirality constraint test, i.e., determine which of the triangulated
// correspondences lie in front of both cameras.
//
// @param cam2_from_cam1  Relative camera transformation.
// @param cam_rays1       First set of corresponding rays.
// @param cam_rays2       Second set of corresponding rays.
// @param points3D        Points that lie in front of both cameras.
bool CheckCheirality(const Rigid3d& cam2_from_cam1,
                     const std::vector<Eigen::Vector3d>& cam_rays1,
                     const std::vector<Eigen::Vector3d>& cam_rays2,
                     std::vector<Eigen::Vector3d>* points3D);

Rigid3d TransformCameraWorld(const Sim3d& new_from_old_world,
                             const Rigid3d& cam_from_world);

}  // namespace colmap
