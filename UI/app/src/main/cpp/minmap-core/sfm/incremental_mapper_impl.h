#pragma once

#include "incremental_mapper.h"
#include "observation_manager.h"
#include "../scene/database.h"
#include "../scene/database_cache.h"
#include "../scene/reconstruction.h"
#include <PoseLib/alignment.h>

namespace colmap {

// Algorithm class for incremental mapper to make it easier to extend
class IncrementalMapperImpl {
 public:
  // Find seed images for incremental reconstruction. Suitable seed images have
  // a large number of correspondences and have camera calibration priors. The
  // returned list is ordered such that most suitable images are in the front.
  static std::vector<image_t> FindFirstInitialImage(
      const IncrementalMapper::Options& options,
      const CorrespondenceGraph& correspondence_graph,
      const Reconstruction& reconstruction,
      const std::unordered_map<image_t, size_t>& init_num_reg_trials,
      const std::unordered_map<image_t, size_t>& num_registrations);

  // For a given first seed image, find other images that are connected to the
  // first image. Suitable second images have a large number of correspondences
  // to the first image and have camera calibration priors. The returned list is
  // ordered such that most suitable images are in the front.
  static std::vector<image_t> FindSecondInitialImage(
      const IncrementalMapper::Options& options,
      image_t image_id1,
      const CorrespondenceGraph& correspondence_graph,
      const Reconstruction& reconstruction,
      const std::unordered_map<image_t, size_t>& num_registrations);

  // Implement IncrementalMapper::FindInitialImagePair
  static bool FindInitialImagePair(
      const IncrementalMapper::Options& options,
      const DatabaseCache& database_cache,
      const Reconstruction& reconstruction,
      const std::unordered_map<image_t, size_t>& init_num_reg_trials,
      const std::unordered_map<image_t, size_t>& num_registrations,
      std::unordered_set<image_pair_t>& init_image_pairs,
      image_t& image_id1,
      image_t& image_id2,
      Rigid3d& cam2_from_cam1);

  // Implement IncrementalMapper::FindNextImages
  static std::vector<image_t> FindNextImages(
      const IncrementalMapper::Options& options,
      const ObservationManager& obs_manager,
      const std::unordered_set<image_t>& filtered_images,
      std::unordered_map<image_t, size_t>& num_reg_trials);

  // Implement IncrementalMapper::FindLocalBundle
  static std::vector<image_t> FindLocalBundle(
      const IncrementalMapper::Options& options,
      image_t image_id,
      const Reconstruction& reconstruction);

  // Implement IncrementalMapper::EstimateInitialTwoViewGeometry
  static bool EstimateInitialTwoViewGeometry(
      const IncrementalMapper::Options& options,
      const DatabaseCache& database_cache,
      image_t image_id1,
      image_t image_id2,
      Rigid3d& cam2_from_cam1);
};

}  // namespace colmap
