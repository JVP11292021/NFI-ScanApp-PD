#ifndef MINMAP_MODEL_H
#define MINMAP_MODEL_H

#include "minmap_defs.hpp"

#include <filesystem>
#include <string>


MM_NS_B

int RunModelConverter(
    const std::filesystem::path& input_path,
    const std::filesystem::path& output_path,
    const std::string& output_type,
    bool skip_distortion = false);

MM_NS_E

#endif // MINMAP_MODEL_H