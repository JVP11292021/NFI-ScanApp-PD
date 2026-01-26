#pragma once

#include "../feature/types.h"
#include "../sensor/bitmap.h"

namespace colmap {

class FeatureExtractor {
 public:
  virtual ~FeatureExtractor() = default;

  virtual bool Extract(const Bitmap& bitmap,
                       FeatureKeypoints* keypoints,
                       FeatureDescriptors* descriptors) = 0;
};

}  // namespace colmap
