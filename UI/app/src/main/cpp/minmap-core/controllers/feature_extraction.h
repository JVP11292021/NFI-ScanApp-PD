#pragma once

#include "image_reader.h"
#include "../feature/sift.h"
#include "../util/threading.h"

namespace colmap {

// Reads images from a folder, extracts features, and writes them to database.
std::unique_ptr<Thread> CreateFeatureExtractorController(
    const std::string& database_path,
    const ImageReaderOptions& reader_options,
    const SiftExtractionOptions& sift_options);

// Import features from text files. Each image must have a corresponding text
// file with the same name and an additional ".txt" suffix.
std::unique_ptr<Thread> CreateFeatureImporterController(
    const std::string& database_path,
    const ImageReaderOptions& reader_options,
    const std::string& import_path);

}  // namespace colmap
