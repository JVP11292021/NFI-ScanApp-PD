#pragma once

#include "../estimators/two_view_geometry.h"
#include "../feature/pairing.h"
#include "../feature/sift.h"
#include "../util/threading.h"

#include <memory>
#include <string>

namespace colmap {

// Exhaustively match images by processing each block in the exhaustive match
// matrix in one batch:
//
// +----+----+-----------------> images[i]
// |#000|0000|
// |1#00|1000| <- Above the main diagonal, the block diagonal is not matched
// |11#0|1100|                                                             ^
// |111#|1110|                                                             |
// +----+----+                                                             |
// |1000|#000|\                                                            |
// |1100|1#00| \ One block                                                 |
// |1110|11#0| / of image pairs                                            |
// |1111|111#|/                                                            |
// +----+----+                                                             |
// |  ^                                                                    |
// |  |                                                                    |
// | Below the main diagonal, the block diagonal is matched <--------------+
// |
// v
// images[i]
//
// Pairs will only be matched if 1, to avoid duplicate pairs. Pairs with #
// are on the main diagonal and denote pairs of the same image.
std::unique_ptr<Thread> CreateExhaustiveFeatureMatcher(
    const ExhaustiveMatchingOptions& options,
    const SiftMatchingOptions& matching_options,
    const TwoViewGeometryOptions& geometry_options,
    const std::string& database_path);

// Match each image against its nearest neighbors using a vocabulary tree.
//std::unique_ptr<Thread> CreateVocabTreeFeatureMatcher(
//    const VocabTreeMatchingOptions& options,
//    const SiftMatchingOptions& matching_options,
//    const TwoViewGeometryOptions& geometry_options,
//    const std::string& database_path);

// Sequentially match images within neighborhood:
//
// +-------------------------------+-----------------------> images[i]
//                      ^          |           ^
//                      |   Current image[i]   |
//                      |          |           |
//                      +----------+-----------+
//                                 |
//                        Match image_i against
//
//                    image_[i - o, i + o]        with o = [1 .. overlap]
//                    image_[i - 2^o, i + 2^o]    (for quadratic overlap)
//
// Sequential order is determined based on the image names in ascending order.
//
// Invoke loop detection if `(i mod loop_detection_period) == 0`, retrieve
// most similar `loop_detection_num_images` images from vocabulary tree,
// and perform matching and verification.
//std::unique_ptr<Thread> CreateSequentialFeatureMatcher(
//    const SequentialMatchingOptions& options,
//    const SiftMatchingOptions& matching_options,
//    const TwoViewGeometryOptions& geometry_options,
//    const std::string& database_path);

// Match images against spatial nearest neighbors using prior location
// information, e.g. provided manually or extracted from EXIF.
//std::unique_ptr<Thread> CreateSpatialFeatureMatcher(
//    const SpatialMatchingOptions& options,
//    const SiftMatchingOptions& matching_options,
//    const TwoViewGeometryOptions& geometry_options,
//    const std::string& database_path);

// Match transitive image pairs in a database with existing feature matches.
// This matcher transitively closes loops/triplets. For example, if image pairs
// A-B and B-C match but A-C has not been matched, then this matcher attempts to
// match A-C. This procedure is performed for multiple iterations.
std::unique_ptr<Thread> CreateTransitiveFeatureMatcher(
    const TransitiveMatchingOptions& options,
    const SiftMatchingOptions& matching_options,
    const TwoViewGeometryOptions& geometry_options,
    const std::string& database_path);

// Match images manually specified in a list of image pairs.
//
// Read matches file with the following format:
//
//    image_name1 image_name2
//    image_name1 image_name3
//    image_name2 image_name3
//    ...
//
std::unique_ptr<Thread> CreateImagePairsFeatureMatcher(
    const ImagePairsMatchingOptions& options,
    const SiftMatchingOptions& matching_options,
    const TwoViewGeometryOptions& geometry_options,
    const std::string& database_path);

// Import feature matches from a text file.
//
// Read matches file with the following format:
//
//      image_name1 image_name2
//      0 1
//      1 2
//      2 3
//      <empty line>
//      image_name1 image_name3
//      0 1
//      1 2
//      2 3
//      ...
//
std::unique_ptr<Thread> CreateFeaturePairsFeatureMatcher(
    const FeaturePairsMatchingOptions& options,
    const SiftMatchingOptions& matching_options,
    const TwoViewGeometryOptions& geometry_options,
    const std::string& database_path);

}  // namespace colmap
