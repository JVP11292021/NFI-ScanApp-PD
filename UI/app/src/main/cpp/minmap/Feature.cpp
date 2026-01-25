#include "Feature.hpp"

#include <controllers/feature_extraction.h>
#include <controllers/option_manager.h>
#include <util/file.h>
#include <util/misc.h>

#define MM_ANDROID_LOG_TAG "MINMAP"

MM_NS_B

bool VerifyCameraParams(const std::string& camera_model,
    const std::string& params) {
    if (!colmap::ExistsCameraModelWithName(camera_model)) {
        LOG(MM_ERROR) << "Camera model does not exist";
        return false;
    }

    const std::vector<double> camera_params = colmap::CSVToVector<double>(params);
    const colmap::CameraModelId camera_model_id = colmap::CameraModelNameToId(camera_model);

    if (camera_params.size() > 0 &&
        !CameraModelVerifyParams(camera_model_id, camera_params)) {
        LOG(MM_ERROR) << "Invalid camera parameters";
        return false;
    }
    return true;
}

bool VerifySiftGPUParams(const bool use_gpu) {
#if !defined(MINMAP_GPU_ENABLED)
    if (use_gpu) {
        LOG(MM_ERROR)
            << "Cannot use Sift GPU without CUDA or OpenGL support; "
            "set SiftExtraction.use_gpu or SiftMatching.use_gpu to false.";
        return false;
    }
#endif
    return true;
}

void UpdateImageReaderOptionsFromCameraMode(colmap::ImageReaderOptions& options, CameraMode mode) {
    switch (mode) {
    case CameraMode::AUTO:
        options.single_camera = false;
        options.single_camera_per_folder = false;
        options.single_camera_per_image = false;
        break;
    case CameraMode::SINGLE:
        options.single_camera = true;
        options.single_camera_per_folder = false;
        options.single_camera_per_image = false;
        break;
    case CameraMode::PER_FOLDER:
        options.single_camera = false;
        options.single_camera_per_folder = true;
        options.single_camera_per_image = false;
        break;
    case CameraMode::PER_IMAGE:
        options.single_camera = false;
        options.single_camera_per_folder = false;
        options.single_camera_per_image = true;
        break;
    }
}

int RunFeatureExtractor(
    const std::filesystem::path& database_path,
    const std::filesystem::path& image_path,
    int camera_mode,
    const std::string& descriptor_normalization,
    const std::string& image_list_path
) {
    LOG(MM_DEBUG) << "Making options manager";
    colmap::OptionManager options;
    *options.database_path = database_path.string();
    *options.image_path = image_path.string();
    options.AddDatabaseOptions();
    options.AddImageOptions();
    options.AddExtractionOptions();

//     Copy reader options
    colmap::ImageReaderOptions reader_options = *options.image_reader;
    reader_options.image_path = *options.image_path;

    // Apply camera mode if specified
    if (camera_mode >= 0) {
        LOG(MM_DEBUG) << "Applying camera mode " << camera_mode;
        UpdateImageReaderOptionsFromCameraMode(reader_options,
            static_cast<CameraMode>(camera_mode));
    }

    // SIFT options
    std::string desc_norm = descriptor_normalization;
    colmap::StringToLower(&desc_norm);
    if (desc_norm == "l1_root") {
        options.sift_extraction->normalization =
            colmap::SiftExtractionOptions::Normalization::L1_ROOT;
    }
    else if (desc_norm == "l2") {
        options.sift_extraction->normalization =
            colmap::SiftExtractionOptions::Normalization::L2;
    }
    else {
        LOG(MM_ERROR) << "Invalid `descriptor_normalization`";
        return EXIT_FAILURE;
    }

    // Optional image list
    if (!image_list_path.empty()) {
        reader_options.image_names = colmap::ReadTextFileLines(image_list_path);
        if (reader_options.image_names.empty()) {
            return EXIT_SUCCESS;
        }
    }

    // Check camera model & parameters
    if (!colmap::ExistsCameraModelWithName(reader_options.camera_model)) {
        LOG(MM_ERROR) << "Camera model does not exist";
        return EXIT_FAILURE;
    }

    if (!VerifyCameraParams(reader_options.camera_model,
        reader_options.camera_params)) {
        return EXIT_FAILURE;
    }

    if (!VerifySiftGPUParams(options.sift_extraction->use_gpu)) {
        return EXIT_FAILURE;
    }

    // Run feature extractor
    auto feature_extractor = colmap::CreateFeatureExtractorController(
        *options.database_path, reader_options, *options.sift_extraction);

    feature_extractor->Start();
    feature_extractor->Wait();

    return EXIT_SUCCESS;
}

int RunExhaustiveMatcher(const std::filesystem::path& database_path) {
    colmap::OptionManager options(false);
    *options.database_path = database_path.string();
    options.AddDatabaseOptions();
    options.AddExhaustiveMatchingOptions();

    options.sift_matching->use_gpu = false;
    if (!VerifySiftGPUParams(options.sift_matching->use_gpu)) {
        return EXIT_FAILURE;
    }

    auto matcher = colmap::CreateExhaustiveFeatureMatcher(
        *options.exhaustive_matching,
        *options.sift_matching,
        *options.two_view_geometry,
        *options.database_path
    );

    matcher->Start();
    matcher->Wait();

    return EXIT_SUCCESS;
}

MM_NS_E