#pragma once

#include "../util/types.h"

#include <vector>

#include <PoseLib/alignment.h>
#include <Eigen/Core>

namespace colmap {

// Essential matrix estimator from corresponding normalized camera ray pairs.
//
// This algorithm solves the 5-Point problem based on the following paper:
//
//    D. Nister, An efficient solution to the five-point relative pose problem,
//    IEEE-T-PAMI, 26(6), 2004.
//    http://citeseerx.ist.psu.edu/viewdoc/summary?doi=10.1.1.86.8769
class EssentialMatrixFivePointEstimator {
 public:
  typedef Eigen::Vector3d X_t;
  typedef Eigen::Vector3d Y_t;
  typedef Eigen::Matrix3d M_t;

  // The minimum number of samples needed to estimate a model.
  static const int kMinNumSamples = 5;

  // Estimate up to 10 possible essential matrix solutions from a set of
  // corresponding camera rays.
  //
  //  The number of corresponding rays must be at least 5.
  //
  // @param cam_rays1  First set of corresponding rays.
  // @param cam_rays2  Second set of corresponding rays.
  //
  // @return           Up to 10 solutions as a vector of 3x3 essential matrices.
  static void Estimate(const std::vector<X_t>& cam_rays1,
                       const std::vector<Y_t>& cam_rays2,
                       std::vector<M_t>* models);

  // Calculate the residuals of a set of corresponding rays and a given
  // essential matrix.
  //
  // Residuals are defined as the squared Sampson error.
  //
  // @param cam_rays1  First set of corresponding rays.
  // @param cam_rays2  Second set of corresponding rays.
  // @param E          3x3 essential matrix.
  // @param residuals  Output vector of residuals.
  static void Residuals(const std::vector<X_t>& cam_rays1,
                        const std::vector<Y_t>& cam_rays2,
                        const M_t& E,
                        std::vector<double>* residuals);
};

// Essential matrix estimator from corresponding normalized camera ray pairs.
//
// This algorithm solves the 8-Point problem based on the following paper:
//
//    Hartley and Zisserman, Multiple View Geometry, algorithm 11.1, page 282.
class EssentialMatrixEightPointEstimator {
 public:
  typedef Eigen::Vector3d X_t;
  typedef Eigen::Vector3d Y_t;
  typedef Eigen::Matrix3d M_t;

  // The minimum number of samples needed to estimate a model.
  static const int kMinNumSamples = 8;

  // Estimate essential matrix solutions from set of corresponding camera rays.

  //
  // The number of corresponding rays must be at least 8.
  //
  // @param cam_rays1  First set of corresponding rays.
  // @param cam_rays2  Second set of corresponding rays.
  static void Estimate(const std::vector<X_t>& cam_rays1,
                       const std::vector<Y_t>& cam_rays2,

                       std::vector<M_t>* models);

  // Calculate the residuals of a set of corresponding rays and a given
  // essential matrix.
  //
  // Residuals are defined as the squared Sampson error.
  //
  // @param cam_rays1  First set of corresponding rays.
  // @param cam_rays2  Second set of corresponding rays.
  // @param E          3x3 essential matrix.
  // @param residuals  Output vector of residuals.
  static void Residuals(const std::vector<X_t>& cam_rays1,
                        const std::vector<Y_t>& cam_rays2,
                        const M_t& E,
                        std::vector<double>* residuals);
};

}  // namespace colmap
