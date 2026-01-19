#include "pairing.h"

//#include "../feature/utils.h"
//#include "../geometry/gps.h"
#include "../util/file.h"
#include "../util/logging.h"
//#include "../util/misc.h"
//#include "../util/timer.h"

#include <fstream>
#include <numeric>
#include <unordered_map>
#include <unordered_set>
#include <vector>

//#include <faiss/IndexFlat.h>
#include <omp.h>

namespace colmap {
namespace {

std::vector<std::pair<image_t, image_t>> ReadImagePairsText(
    const std::string& path,
    const std::unordered_map<std::string, image_t>& image_name_to_image_id) {
  std::ifstream file(path);
  THROW_CHECK_FILE_OPEN(file, path);

  std::string line;
  std::vector<std::pair<image_t, image_t>> image_pairs;
  std::unordered_set<image_pair_t> image_pairs_set;
  while (std::getline(file, line)) {
    StringTrim(&line);

    if (line.empty() || line[0] == '#') {
      continue;
    }

    std::stringstream line_stream(line);

    std::string image_name1;
    std::string image_name2;

    std::getline(line_stream, image_name1, ' ');
    StringTrim(&image_name1);
    std::getline(line_stream, image_name2, ' ');
    StringTrim(&image_name2);

    if (image_name_to_image_id.count(image_name1) == 0) {
      LOG(MM_ERROR) << "Image " << image_name1 << " does not exist.";
      continue;
    }
    if (image_name_to_image_id.count(image_name2) == 0) {
      LOG(MM_ERROR) << "Image " << image_name2 << " does not exist.";
      continue;
    }

    const image_t image_id1 = image_name_to_image_id.at(image_name1);
    const image_t image_id2 = image_name_to_image_id.at(image_name2);
    const image_pair_t image_pair =
        Database::ImagePairToPairId(image_id1, image_id2);
    const bool image_pair_exists = image_pairs_set.insert(image_pair).second;
    if (image_pair_exists) {
      image_pairs.emplace_back(image_id1, image_id2);
    }
  }
  return image_pairs;
}

}  // namespace

bool ExhaustiveMatchingOptions::Check() const {
  CHECK_OPTION_GT(block_size, 1);
  return true;
}

bool TransitiveMatchingOptions::Check() const {
  CHECK_OPTION_GT(batch_size, 0);
  CHECK_OPTION_GT(num_iterations, 0);
  return true;
}

bool ImagePairsMatchingOptions::Check() const {
  CHECK_OPTION_GT(block_size, 0);
  return true;
}

bool FeaturePairsMatchingOptions::Check() const { return true; }

std::vector<std::pair<image_t, image_t>> PairGenerator::AllPairs() {
  std::vector<std::pair<image_t, image_t>> image_pairs;
  while (!this->HasFinished()) {
    std::vector<std::pair<image_t, image_t>> image_pairs_block = this->Next();
    image_pairs.insert(image_pairs.end(),
                       std::make_move_iterator(image_pairs_block.begin()),
                       std::make_move_iterator(image_pairs_block.end()));
  }
  return image_pairs;
}

ExhaustivePairGenerator::ExhaustivePairGenerator(
    const ExhaustiveMatchingOptions& options,
    const std::shared_ptr<FeatureMatcherCache>& cache)
    : options_(options),
      image_ids_(THROW_CHECK_NOTNULL(cache)->GetImageIds()),
      block_size_(static_cast<size_t>(options_.block_size)),
      num_blocks_(static_cast<size_t>(
          std::ceil(static_cast<double>(image_ids_.size()) / block_size_))) {
  THROW_CHECK(options.Check());
  LOG(MM_INFO) << "Generating exhaustive image pairs...";
  const size_t num_pairs_per_block = block_size_ * (block_size_ - 1) / 2;
  image_pairs_.reserve(num_pairs_per_block);
}

ExhaustivePairGenerator::ExhaustivePairGenerator(
    const ExhaustiveMatchingOptions& options,
    const std::shared_ptr<Database>& database)
    : ExhaustivePairGenerator(
          options,
          std::make_shared<FeatureMatcherCache>(
              options.CacheSize(), THROW_CHECK_NOTNULL(database))) {}

void ExhaustivePairGenerator::Reset() {
  start_idx1_ = 0;
  start_idx2_ = 0;
}

bool ExhaustivePairGenerator::HasFinished() const {
  return start_idx1_ >= image_ids_.size();
}

std::vector<std::pair<image_t, image_t>> ExhaustivePairGenerator::Next() {
  image_pairs_.clear();
  if (HasFinished()) {
    return image_pairs_;
  }

  const size_t end_idx1 =
      std::min(image_ids_.size(), start_idx1_ + block_size_) - 1;
  const size_t end_idx2 =
      std::min(image_ids_.size(), start_idx2_ + block_size_) - 1;

  LOG(MM_INFO) << StringPrintf("Matching block [%d/%d, %d/%d]",
                            start_idx1_ / block_size_ + 1,
                            num_blocks_,
                            start_idx2_ / block_size_ + 1,
                            num_blocks_);

  for (size_t idx1 = start_idx1_; idx1 <= end_idx1; ++idx1) {
    for (size_t idx2 = start_idx2_; idx2 <= end_idx2; ++idx2) {
      const size_t block_id1 = idx1 % block_size_;
      const size_t block_id2 = idx2 % block_size_;
      if ((idx1 > idx2 && block_id1 <= block_id2) ||
          (idx1 < idx2 && block_id1 < block_id2)) {  // Avoid duplicate pairs
        image_pairs_.emplace_back(image_ids_[idx1], image_ids_[idx2]);
      }
    }
  }
  start_idx2_ += block_size_;
  if (start_idx2_ >= image_ids_.size()) {
    start_idx2_ = 0;
    start_idx1_ += block_size_;
  }
  return image_pairs_;
}

TransitivePairGenerator::TransitivePairGenerator(
    const TransitiveMatchingOptions& options,
    const std::shared_ptr<FeatureMatcherCache>& cache)
    : options_(options), cache_(cache) {
  THROW_CHECK(options.Check());
}

TransitivePairGenerator::TransitivePairGenerator(
    const TransitiveMatchingOptions& options,
    const std::shared_ptr<Database>& database)
    : TransitivePairGenerator(
          options,
          std::make_shared<FeatureMatcherCache>(
              options.CacheSize(), THROW_CHECK_NOTNULL(database))) {}

void TransitivePairGenerator::Reset() {
  current_iteration_ = 0;
  current_batch_idx_ = 0;
  image_pairs_.clear();
  image_pair_ids_.clear();
}

bool TransitivePairGenerator::HasFinished() const {
  return current_iteration_ >= options_.num_iterations && image_pairs_.empty();
}

std::vector<std::pair<image_t, image_t>> TransitivePairGenerator::Next() {
  if (!image_pairs_.empty()) {
    current_batch_idx_++;
    std::vector<std::pair<image_t, image_t>> batch;
    while (!image_pairs_.empty() &&
           static_cast<int>(batch.size()) < options_.batch_size) {
      batch.push_back(image_pairs_.back());
      image_pairs_.pop_back();
    }
    LOG(MM_INFO) << StringPrintf(
        "Matching batch [%d/%d]", current_batch_idx_, current_num_batches_);
    return batch;
  }

  if (current_iteration_ >= options_.num_iterations) {
    return {};
  }

  current_batch_idx_ = 0;
  current_num_batches_ = 0;
  current_iteration_++;

  LOG(MM_INFO) << StringPrintf(
      "Iteration [%d/%d]", current_iteration_, options_.num_iterations);

  std::vector<std::pair<image_pair_t, int>> existing_pair_ids_and_num_inliers;
  cache_->AccessDatabase(
      [&existing_pair_ids_and_num_inliers](Database& database) {
        existing_pair_ids_and_num_inliers =
            database.ReadTwoViewGeometryNumInliers();
      });

  std::map<image_t, std::vector<image_t>> adjacency;
  for (const auto& [pair_id, _] : existing_pair_ids_and_num_inliers) {
    const auto [image_id1, image_id2] = Database::PairIdToImagePair(pair_id);
    adjacency[image_id1].push_back(image_id2);
    adjacency[image_id2].push_back(image_id1);
    image_pair_ids_.insert(pair_id);
  }

  for (const auto& image : adjacency) {
    const auto image_id1 = image.first;
    for (const auto& image_id2 : image.second) {
      const auto it = adjacency.find(image_id2);
      if (it == adjacency.end()) {
        continue;
      }
      for (const auto& image_id3 : it->second) {
        if (image_id1 == image_id3) {
          continue;
        }
        const auto image_pair_id =
            Database::ImagePairToPairId(image_id1, image_id3);
        if (image_pair_ids_.count(image_pair_id) != 0) {
          continue;
        }
        image_pairs_.emplace_back(std::minmax(image_id1, image_id3));
        image_pair_ids_.insert(image_pair_id);
      }
    }
  }

  current_num_batches_ =
      std::ceil(static_cast<double>(image_pairs_.size()) / options_.batch_size);

  return Next();
}

ImportedPairGenerator::ImportedPairGenerator(
    const ImagePairsMatchingOptions& options,
    const std::shared_ptr<FeatureMatcherCache>& cache)
    : options_(options) {
  THROW_CHECK(options.Check());

  LOG(MM_INFO) << "Importing image pairs...";
  const std::vector<image_t> image_ids = cache->GetImageIds();
  std::unordered_map<std::string, image_t> image_name_to_image_id;
  image_name_to_image_id.reserve(image_ids.size());
  for (const auto image_id : image_ids) {
    const auto& image = cache->GetImage(image_id);
    image_name_to_image_id.emplace(image.Name(), image_id);
  }
  image_pairs_ =
      ReadImagePairsText(options_.match_list_path, image_name_to_image_id);
  block_image_pairs_.reserve(options_.block_size);
}

ImportedPairGenerator::ImportedPairGenerator(
    const ImagePairsMatchingOptions& options,
    const std::shared_ptr<Database>& database)
    : ImportedPairGenerator(
          options,
          std::make_shared<FeatureMatcherCache>(
              options.CacheSize(), THROW_CHECK_NOTNULL(database))) {}

void ImportedPairGenerator::Reset() { pair_idx_ = 0; }

bool ImportedPairGenerator::HasFinished() const {
  return pair_idx_ >= image_pairs_.size();
}

std::vector<std::pair<image_t, image_t>> ImportedPairGenerator::Next() {
  block_image_pairs_.clear();
  if (HasFinished()) {
    return block_image_pairs_;
  }

  LOG(MM_INFO) << StringPrintf("Matching block [%d/%d]",
                            pair_idx_ / options_.block_size + 1,
                            image_pairs_.size() / options_.block_size + 1);

  const size_t block_end =
      std::min(pair_idx_ + options_.block_size, image_pairs_.size());
  for (size_t j = pair_idx_; j < block_end; ++j) {
    block_image_pairs_.push_back(image_pairs_[j]);
  }
  pair_idx_ += options_.block_size;
  return block_image_pairs_;
}

}  // namespace colmap
