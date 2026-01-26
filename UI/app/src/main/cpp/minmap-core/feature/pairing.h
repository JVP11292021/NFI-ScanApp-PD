#pragma once

#include "../feature/matcher.h"
#include "../scene/database.h"
#include "../util/types.h"

#include <unordered_set>

namespace colmap {

struct ExhaustiveMatchingOptions {
  // Block size, i.e. number of images to simultaneously load into memory.
  int block_size = 50;

  bool Check() const;

  inline size_t CacheSize() const { return block_size; }
};

struct TransitiveMatchingOptions {
  // The maximum number of image pairs to process in one batch.
  int batch_size = 1000;

  // The number of transitive closure iterations.
  int num_iterations = 3;

  bool Check() const;

  inline size_t CacheSize() const { return 2 * batch_size; }
};

struct ImagePairsMatchingOptions {
  // Number of image pairs to match in one batch.
  int block_size = 1225;

  // Path to the file with the matches.
  std::string match_list_path = "";

  bool Check() const;

  inline size_t CacheSize() const { return block_size; }
};

struct FeaturePairsMatchingOptions {
  // Whether to geometrically verify the given matches.
  bool verify_matches = true;

  // Path to the file with the matches.
  std::string match_list_path = "";

  bool Check() const;
};

class PairGenerator {
 public:
  virtual ~PairGenerator() = default;

  virtual void Reset() = 0;

  virtual bool HasFinished() const = 0;

  virtual std::vector<std::pair<image_t, image_t>> Next() = 0;

  std::vector<std::pair<image_t, image_t>> AllPairs();
};

class ExhaustivePairGenerator : public PairGenerator {
 public:
  using PairOptions = ExhaustiveMatchingOptions;

  ExhaustivePairGenerator(const ExhaustiveMatchingOptions& options,
                          const std::shared_ptr<FeatureMatcherCache>& cache);

  ExhaustivePairGenerator(const ExhaustiveMatchingOptions& options,
                          const std::shared_ptr<Database>& database);

  void Reset() override;

  bool HasFinished() const override;

  std::vector<std::pair<image_t, image_t>> Next() override;

 private:
  const ExhaustiveMatchingOptions options_;
  const std::vector<image_t> image_ids_;
  const size_t block_size_;
  const size_t num_blocks_;
  size_t start_idx1_ = 0;
  size_t start_idx2_ = 0;
  std::vector<std::pair<image_t, image_t>> image_pairs_;
};

class TransitivePairGenerator : public PairGenerator {
 public:
  using PairOptions = TransitiveMatchingOptions;

  TransitivePairGenerator(const TransitiveMatchingOptions& options,
                          const std::shared_ptr<FeatureMatcherCache>& cache);

  TransitivePairGenerator(const TransitiveMatchingOptions& options,
                          const std::shared_ptr<Database>& database);

  void Reset() override;

  bool HasFinished() const override;

  std::vector<std::pair<image_t, image_t>> Next() override;

 private:
  const TransitiveMatchingOptions options_;
  const std::shared_ptr<FeatureMatcherCache> cache_;
  int current_iteration_ = 0;
  int current_batch_idx_ = 0;
  int current_num_batches_ = 0;
  std::vector<std::pair<image_t, image_t>> image_pairs_;
  std::unordered_set<image_pair_t> image_pair_ids_;
};

class ImportedPairGenerator : public PairGenerator {
 public:
  using PairOptions = ImagePairsMatchingOptions;

  ImportedPairGenerator(const ImagePairsMatchingOptions& options,
                        const std::shared_ptr<FeatureMatcherCache>& cache);

  ImportedPairGenerator(const ImagePairsMatchingOptions& options,
                        const std::shared_ptr<Database>& database);

  void Reset() override;

  bool HasFinished() const override;

  std::vector<std::pair<image_t, image_t>> Next() override;

 private:
  const ImagePairsMatchingOptions options_;
  std::vector<std::pair<image_t, image_t>> image_pairs_;
  std::vector<std::pair<image_t, image_t>> block_image_pairs_;
  size_t pair_idx_ = 0;
};

}  // namespace colmap
