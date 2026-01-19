#ifndef MINMAP_SFM_H
#define MINMAP_SFM_H

#include "minmap_defs.hpp"

#include <filesystem>

#include <controllers/incremental_pipeline.h>
#include <scene/reconstruction.h>

MM_NS_B

int RunMapper(const std::filesystem::path& database_path,
    const std::filesystem::path& image_path,
    const std::filesystem::path& output_path,
    const std::string& input_path = "",
    const std::string& image_list_path = "",
    bool fix_existing_frames = true);

MM_NS_E

#endif // MINMAP_SFM_H