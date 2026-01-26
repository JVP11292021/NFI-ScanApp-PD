#pragma once

#include "../feature/index.h"
#include "../feature/types.h"
#include "../geometry/gps.h"
#include "../scene/camera.h"
#include "../scene/database.h"
#include "../scene/image.h"
#include "../scene/two_view_geometry.h"
#include "../util/cache.h"
#include "../util/types.h"

#include <memory>
#include <mutex>
#include <unordered_map>

namespace colmap {

class FeatureMatcher {
 public:
  virtual ~FeatureMatcher() = default;

  struct Image {
    // Unique identifier for the image. Allows a matcher to cache some
    // computations per image in consecutive calls to matching.
    image_t image_id = kInvalidImageId;
    // Used for both normal and guided matching.
    std::shared_ptr<const FeatureDescriptors> descriptors;
    // Only used for guided matching.
    std::shared_ptr<const FeatureKeypoints> keypoints;
  };

  virtual void Match(const Image& image1,
                     const Image& image2,
                     FeatureMatches* matches) = 0;

  virtual void MatchGuided(double max_error,
                           const Image& image1,
                           const Image& image2,
                           TwoViewGeometry* two_view_geometry) = 0;
};

// Cache for feature matching to minimize database access during matching.
class FeatureMatcherCache {
 public:
  FeatureMatcherCache(size_t cache_size,
                      const std::shared_ptr<Database>& database);

  // Executes a function that accesses the database. This function is thread
  // safe and ensures that only one function can access the database at a time.
  void AccessDatabase(const std::function<void(Database& database)>& func);

  const Camera& GetCamera(camera_t camera_id);
  const Frame& GetFrame(frame_t frame_id);
  const Image& GetImage(image_t image_id);
  std::shared_ptr<FeatureKeypoints> GetKeypoints(image_t image_id);
  std::shared_ptr<FeatureDescriptors> GetDescriptors(image_t image_id);
  FeatureMatches GetMatches(image_t image_id1, image_t image_id2);
  std::vector<frame_t> GetFrameIds();
  std::vector<image_t> GetImageIds();
  ThreadSafeLRUCache<image_t, FeatureDescriptorIndex>&
  GetFeatureDescriptorIndexCache();

  bool ExistsKeypoints(image_t image_id);
  bool ExistsDescriptors(image_t image_id);

  bool ExistsMatches(image_t image_id1, image_t image_id2);
  bool ExistsInlierMatches(image_t image_id1, image_t image_id2);

  void WriteMatches(image_t image_id1,
                    image_t image_id2,
                    const FeatureMatches& matches);
  void WriteTwoViewGeometry(image_t image_id1,
                            image_t image_id2,
                            const TwoViewGeometry& two_view_geometry);

  void DeleteMatches(image_t image_id1, image_t image_id2);
  void DeleteInlierMatches(image_t image_id1, image_t image_id2);

  size_t MaxNumKeypoints();

 private:
  void MaybeLoadCameras();
  void MaybeLoadFrames();
  void MaybeLoadImages();
  void MaybeLoadPosePriors();

  const size_t cache_size_;
  const std::shared_ptr<Database> database_;
  std::mutex database_mutex_;
  std::unique_ptr<std::unordered_map<camera_t, Camera>> cameras_cache_;
  std::unique_ptr<std::unordered_map<frame_t, Frame>> frames_cache_;
  std::unique_ptr<std::unordered_map<image_t, Image>> images_cache_;
  std::unique_ptr<std::unordered_map<image_t, PosePrior>> pose_priors_cache_;
  std::unique_ptr<ThreadSafeLRUCache<image_t, FeatureKeypoints>>
      keypoints_cache_;
  std::unique_ptr<ThreadSafeLRUCache<image_t, FeatureDescriptors>>
      descriptors_cache_;
  std::unique_ptr<ThreadSafeLRUCache<image_t, bool>> keypoints_exists_cache_;
  std::unique_ptr<ThreadSafeLRUCache<image_t, bool>> descriptors_exists_cache_;
  ThreadSafeLRUCache<image_t, FeatureDescriptorIndex> descriptor_index_cache_;
};

}  // namespace colmap
