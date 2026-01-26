//
// Created by jessy on 1/25/2026.
//

#ifndef IPMEDTH_NFI_RECONSTRUCTION_ENGINE_H
#define IPMEDTH_NFI_RECONSTRUCTION_ENGINE_H

#include "Feature.hpp"
#include "MapModel.hpp"
#include "SfM.hpp"
#include "minmap_defs.hpp"

MM_NS_B

class ReconstructionEngine {
public:
    explicit ReconstructionEngine(std::string& datasetPath, std::string& databasePath);

    std::int8_t extractFeatures(
            int camera_mode = -1,
            const std::string& descriptor_normalization = "l1_root",
            const std::string& image_list_path = "");
    std::int8_t matchFeatures();
    std::int8_t reconstruct(
            const std::string& output_path,
            const std::string& input_path = "",
            const std::string& image_list_path = "",
            bool fix_existing_frames = true);
    std::int8_t mapModel(
            const std::string& input_path,
            const std::string& output_path,
            const std::string& output_type,
            bool skip_distortion = false);

private:
    std::string datasetPath;
    std::string databasePath;
};

MM_NS_E

#endif //IPMEDTH_NFI_RECONSTRUCTION_ENGINE_H
