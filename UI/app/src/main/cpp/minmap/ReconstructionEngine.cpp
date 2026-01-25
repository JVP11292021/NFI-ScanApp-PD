//
// Created by jessy on 1/25/2026.
//

#include "ReconstructionEngine.hpp"

#include <util/logging.h>

#define MM_ANDROID_LOG_TAG "MINMAP"

MM_NS_B

ReconstructionEngine::ReconstructionEngine(std::string& datasetPath, std::string& databasePath) {

}

std::int8_t ReconstructionEngine::extractFeatures(
        int camera_mode,
        const std::string& descriptor_normalization,
        const std::string& image_list_path
) {
    if (RunFeatureExtractor(
            this->databasePath,
            this->datasetPath,
            camera_mode,
            descriptor_normalization,
            image_list_path) == EXIT_FAILURE)
    {
        LOG(MM_ERROR) << "Feature extraction failed";
        return EXIT_FAILURE;
    }

    LOG(MM_INFO) << "Feature extraction succeeded";
    return EXIT_SUCCESS;
}

std::int8_t ReconstructionEngine::matchFeatures() {
    if (RunExhaustiveMatcher(this->databasePath) == EXIT_FAILURE) {
        LOG(MM_ERROR) << "Feature matching failed";
        return EXIT_FAILURE;
    }

    LOG(MM_INFO) << "Feature matching succeeded";
    return EXIT_SUCCESS;
}

std::int8_t ReconstructionEngine::reconstruct(
        const std::string& output_path,
        const std::string& input_path,
        const std::string& image_list_path,
        bool fix_existing_frames
) {
    if (RunMapper(
            this->databasePath,
            this->datasetPath,
            output_path,
            input_path,
            image_list_path,
            fix_existing_frames) == EXIT_FAILURE)
    {
        LOG(MM_ERROR) << "Reconstruction failed";
        return EXIT_FAILURE;
    }

    LOG(MM_INFO) << "Reconstruction succeeded";
    return EXIT_SUCCESS;
}

std::int8_t ReconstructionEngine::mapModel(
        const std::string &input_path,
        const std::string &output_path,
        const std::string &output_type,
        bool skip_distortion
) {
    if (RunModelConverter(
            input_path,
            output_path,
            output_type,
            skip_distortion) == EXIT_FAILURE)
    {
        LOG(MM_ERROR) << "Model conversion failed";
        return EXIT_FAILURE;
    }

    LOG(MM_INFO) << "Model conversion succeeded";
    return EXIT_SUCCESS;
}

MM_NS_E