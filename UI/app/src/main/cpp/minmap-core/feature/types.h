#pragma once

#include "../util/types.h"

#include <vector>

#include <PoseLib/alignment.h>
#include <Eigen/Core>

namespace colmap {

struct FeatureKeypoint {
  FeatureKeypoint();
  FeatureKeypoint(float x, float y);
  FeatureKeypoint(float x, float y, float scale, float orientation);
  FeatureKeypoint(float x, float y, float a11, float a12, float a21, float a22);

  static FeatureKeypoint FromShapeParameters(float x,
                                             float y,
                                             float scale_x,
                                             float scale_y,
                                             float orientation,
                                             float shear);

  // Rescale the feature location and shape size by the given scale factor.
  void Rescale(float scale);
  void Rescale(float scale_x, float scale_y);

  // Compute shape parameters from affine shape.
  float ComputeScale() const;
  float ComputeScaleX() const;
  float ComputeScaleY() const;
  float ComputeOrientation() const;
  float ComputeShear() const;

  // Location of the feature, with the origin at the upper left image corner,
  // i.e. the upper left pixel has the coordinate (0.5, 0.5).
  float x;
  float y;

  // Affine shape of the feature.
  float a11;
  float a12;
  float a21;
  float a22;
};

typedef Eigen::Matrix<uint8_t, 1, Eigen::Dynamic, Eigen::RowMajor>
    FeatureDescriptor;
typedef std::vector<FeatureKeypoint> FeatureKeypoints;
typedef Eigen::Matrix<uint8_t, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>
    FeatureDescriptors;
typedef Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>
    FeatureDescriptorsFloat;

struct FeatureMatch {
  FeatureMatch()
      : point2D_idx1(kInvalidPoint2DIdx), point2D_idx2(kInvalidPoint2DIdx) {}
  FeatureMatch(const point2D_t point2D_idx1, const point2D_t point2D_idx2)
      : point2D_idx1(point2D_idx1), point2D_idx2(point2D_idx2) {}

  // Feature index in first image.
  point2D_t point2D_idx1 = kInvalidPoint2DIdx;

  // Feature index in second image.
  point2D_t point2D_idx2 = kInvalidPoint2DIdx;
};

typedef std::vector<FeatureMatch> FeatureMatches;

}  // namespace colmap
