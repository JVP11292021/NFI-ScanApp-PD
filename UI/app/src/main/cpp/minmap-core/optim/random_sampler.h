#pragma once

#include "sampler.h"

namespace colmap {

// Random sampler for RANSAC-based methods.
//
// Note that a separate sampler should be instantiated per thread.
class RandomSampler : public Sampler {
 public:
  explicit RandomSampler(size_t num_samples);

  void Initialize(size_t total_num_samples) override;

  size_t MaxNumSamples() override;

  void Sample(std::vector<size_t>* sampled_idxs) override;

 private:
  const size_t num_samples_;
  std::vector<size_t> sample_idxs_;
};

}  // namespace colmap
