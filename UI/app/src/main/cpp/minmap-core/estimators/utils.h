#pragma once

#include "../util/types.h"

#include <vector>

#include <PoseLib/alignment.h>
#include <Eigen/Core>

namespace colmap {

// Center and normalize image points.
//
// The points are transformed in a two-step procedure that is expressed
// as a transformation matrix. The matrix of the resulting points is usually
// better conditioned than the matrix of the original points.
//
// Center the image points, such that the new coordinate system has its
// origin at the centroid of the image points.
//
// Normalize the image points, such that the mean distance from the points
// to the coordinate system is sqrt(2).
//
// @param points            Image coordinates.
// @param normed_points     Transformed image coordinates.
// @param normed_from_orig  3x3 transformation matrix.
void CenterAndNormalizeImagePoints(const std::vector<Eigen::Vector2d>& points,
                                   std::vector<Eigen::Vector2d>* normed_points,
                                   Eigen::Matrix3d* normed_from_orig);

// Calculate the residuals of a set of corresponding points and a given
// fundamental or essential matrix.
//
// Residuals are defined as the squared Sampson error.
//
// @param points1     Corresponding points/rays.
// @param points2     Corresponding points/rays.
// @param E           3x3 fundamental or essential matrix.
// @param residuals   Output vector of residuals.
void ComputeSquaredSampsonError(const std::vector<Eigen::Vector2d>& points1,
                                const std::vector<Eigen::Vector2d>& points2,
                                const Eigen::Matrix3d& E,
                                std::vector<double>* residuals);
void ComputeSquaredSampsonError(const std::vector<Eigen::Vector3d>& rays1,
                                const std::vector<Eigen::Vector3d>& rays2,
                                const Eigen::Matrix3d& E,
                                std::vector<double>* residuals);

}  // namespace colmap
