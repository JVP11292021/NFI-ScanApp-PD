#pragma once

#include <fstream>
#include <sstream>
#include <functional>
#include <unordered_map>
#include <memory>

namespace colmap {

struct ImageReaderOptions;
struct SiftExtractionOptions;
struct SiftMatchingOptions;
struct TwoViewGeometryOptions;
struct ExhaustiveMatchingOptions;
struct TransitiveMatchingOptions;
struct ImagePairsMatchingOptions;
struct IncrementalPipelineOptions;

class OptionManager {
 public:
  explicit OptionManager(bool add_project_options = true);

  // Create "optimal" set of options for different reconstruction scenarios.
  // Note that the existing options are modified, so if your parameters are
  // already low quality, they will be further modified.
  void ModifyForIndividualData();
  void ModifyForVideoData();

  // Create "optimal" set of options for different quality settings.
  // Note that the existing options are modified, so if your parameters are
  // already low quality, they will be further degraded.
  void ModifyForLowQuality();
  void ModifyForMediumQuality();
  void ModifyForHighQuality();
  void ModifyForExtremeQuality();

  void AddAllOptions();
  void AddLogOptions();
  void AddRandomOptions();
  void AddDatabaseOptions();
  void AddImageOptions();
  void AddExtractionOptions();
  void AddMatchingOptions();
  void AddExhaustiveMatchingOptions();
  void AddTransitiveMatchingOptions();
  void AddImagePairsMatchingOptions();
  void AddMapperOptions();

  void Reset();
  void ResetOptions(bool reset_paths);

  bool Check();

  bool Read(const std::string& path);
  bool ReRead(const std::string& path);
  void Write(const std::string& path) const;

  std::shared_ptr<std::string> project_path;
  std::shared_ptr<std::string> database_path;
  std::shared_ptr<std::string> image_path;

  std::shared_ptr<ImageReaderOptions> image_reader;
  std::shared_ptr<SiftExtractionOptions> sift_extraction;

  std::shared_ptr<SiftMatchingOptions> sift_matching;
  std::shared_ptr<TwoViewGeometryOptions> two_view_geometry;
  std::shared_ptr<ExhaustiveMatchingOptions> exhaustive_matching;
  std::shared_ptr<TransitiveMatchingOptions> transitive_matching;
  std::shared_ptr<ImagePairsMatchingOptions> image_pairs_matching;

  std::shared_ptr<IncrementalPipelineOptions> mapper;

 private:
    struct Entry {
        std::function<void(const std::string&)> set;
        std::function<std::string()> get;
    };

    std::unordered_map<std::string, Entry> entries_;

    template<typename T>
    void Register(const std::string& name, T* ptr);

  bool added_log_options_;
  bool added_random_options_;
  bool added_database_options_;
  bool added_image_options_;
  bool added_extraction_options_;
  bool added_match_options_;
  bool added_exhaustive_match_options_;
  bool added_sequential_match_options_;
  bool added_vocab_tree_match_options_;
  bool added_spatial_match_options_;
  bool added_transitive_match_options_;
  bool added_image_pairs_match_options_;
  bool added_ba_options_;
  bool added_mapper_options_;
  bool added_patch_match_stereo_options_;
  bool added_stereo_fusion_options_;
  bool added_poisson_meshing_options_;
  bool added_delaunay_meshing_options_;
  bool added_render_options_;
};

////////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////////

template<typename T>
void OptionManager::Register(const std::string& name, T* ptr) {
    entries_[name] = {
      [ptr](const std::string& v) {
        std::stringstream ss(v); ss >> *ptr;
      },
      [ptr]() {
        std::stringstream ss; ss << *ptr; return ss.str();
      }
    };
}

}  // namespace colmap
