#include "Feature.hpp"

#include <controllers/feature_extraction.h> // Dependencies: sift.h, extractor.h, matcher.h, image_reader.h, threading.h, index.h, types.h, gps.h, camera.h, database.h, image.h, two_view_geoem try.h, cache.h, types.h
#include <controllers/option_manager.h> // Needs te be refactored
//#include "colmap/feature/sift.h" // Has multiple dependencies
//#include "colmap/feature/utils.h" // Dependencies: types.h
//#include "colmap/sensor/models.h" // Dependencies: math.h, enum_utils.h, types.h (all colmap)
#include <util/file.h>  // Dependencies: endian.h, types.h, logging.h (all colmap)
#include <util/misc.h> // Dependencies: logging.h
//#include "colmap/util/threading.h" // Dependencies: timer.h

MM_NS_B

bool VerifyCameraParams(const std::string& camera_model,
    const std::string& params) {
    if (!colmap::ExistsCameraModelWithName(camera_model)) {
        LOG(ERROR) << "Camera model does not exist";
        return false;
    }

    const std::vector<double> camera_params = colmap::CSVToVector<double>(params);
    const colmap::CameraModelId camera_model_id = colmap::CameraModelNameToId(camera_model);

    if (camera_params.size() > 0 &&
        !CameraModelVerifyParams(camera_model_id, camera_params)) {
        LOG(ERROR) << "Invalid camera parameters";
        return false;
    }
    return true;
}

bool VerifySiftGPUParams(const bool use_gpu) {
#if !defined(COLMAP_GPU_ENABLED)
    if (use_gpu) {
        LOG(ERROR)
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
    colmap::OptionManager options;
    *options.database_path = database_path.string();
    *options.image_path = image_path.string();
    options.AddDatabaseOptions();
    options.AddImageOptions();
    options.AddExtractionOptions();

    // Copy reader options
    //colmap::ImageReaderOptions reader_options = *options.image_reader;
    //reader_options.image_path = *options.image_path;

    //// Apply camera mode if specified
    //if (camera_mode >= 0) {
    //    UpdateImageReaderOptionsFromCameraMode(reader_options,
    //        static_cast<CameraMode>(camera_mode));
    //}

    //// SIFT options
    //options.sift_extraction->use_gpu = false;
    //std::string desc_norm = descriptor_normalization;
    //colmap::StringToLower(&desc_norm);
    //if (desc_norm == "l1_root") {
    //    options.sift_extraction->normalization =
    //        colmap::SiftExtractionOptions::Normalization::L1_ROOT;
    //}
    //else if (desc_norm == "l2") {
    //    options.sift_extraction->normalization =
    //        colmap::SiftExtractionOptions::Normalization::L2;
    //}
    //else {
    //    LOG(ERROR) << "Invalid `descriptor_normalization`";
    //    return EXIT_FAILURE;
    //}

    //// Optional image list
    //if (!image_list_path.empty()) {
    //    reader_options.image_names = colmap::ReadTextFileLines(image_list_path);
    //    if (reader_options.image_names.empty()) {
    //        return EXIT_SUCCESS;
    //    }
    //}

    //// Check camera model & parameters
    //if (!colmap::ExistsCameraModelWithName(reader_options.camera_model)) {
    //    LOG(ERROR) << "Camera model does not exist";
    //    return EXIT_FAILURE;
    //}

    //if (!VerifyCameraParams(reader_options.camera_model,
    //    reader_options.camera_params)) {
    //    return EXIT_FAILURE;
    //}

    //if (!VerifySiftGPUParams(options.sift_extraction->use_gpu)) {
    //    return EXIT_FAILURE;
    //}

    //// Run feature extractor
    //auto feature_extractor = colmap::CreateFeatureExtractorController(
    //    *options.database_path, reader_options, *options.sift_extraction);

    //feature_extractor->Start();
    //feature_extractor->Wait();

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

//int RunFeatureImporter(int argc, char** argv) {
//    std::string import_path;
//    std::string image_list_path;
//    int camera_mode = -1;
//
//    colmap::OptionManager options;
//    options.AddDatabaseOptions();
//    options.AddImageOptions();
//    options.AddDefaultOption("camera_mode", &camera_mode);
//    options.AddRequiredOption("import_path", &import_path);
//    options.AddDefaultOption("image_list_path", &image_list_path);
//    options.AddExtractionOptions();
//    options.Parse(argc, argv);
//
//    colmap::ImageReaderOptions reader_options = *options.image_reader;
//    reader_options.image_path = *options.image_path;
//
//    if (camera_mode >= 0) {
//        UpdateImageReaderOptionsFromCameraMode(reader_options,
//            (CameraMode)camera_mode);
//    }
//
//    if (!image_list_path.empty()) {
//        reader_options.image_names = colmap::ReadTextFileLines(image_list_path);
//        if (reader_options.image_names.empty()) {
//            return EXIT_SUCCESS;
//        }
//    }
//
//    if (!VerifyCameraParams(reader_options.camera_model,
//        reader_options.camera_params)) {
//        return EXIT_FAILURE;
//    }
//
//    auto feature_importer = colmap::CreateFeatureImporterController(
//        *options.database_path, reader_options, import_path);
//    feature_importer->Start();
//    feature_importer->Wait();
//
//    return EXIT_SUCCESS;
//}
//
//
//int RunMatchesImporter(int argc, char** argv) {
//    std::string match_list_path;
//    std::string match_type = "pairs";
//
//    colmap::OptionManager options;
//    options.AddDatabaseOptions();
//    options.AddRequiredOption("match_list_path", &match_list_path);
//    options.AddDefaultOption(
//        "match_type", &match_type, "{'pairs', 'raw', 'inliers'}");
//    options.AddMatchingOptions();
//    options.Parse(argc, argv);
//
//    if (!VerifySiftGPUParams(options.sift_matching->use_gpu)) {
//        return EXIT_FAILURE;
//    }
//
//    std::unique_ptr<colmap::Thread> matcher;
//    if (match_type == "pairs") {
//        colmap::ImagePairsMatchingOptions matcher_options;
//        matcher_options.match_list_path = match_list_path;
//        matcher = CreateImagePairsFeatureMatcher(matcher_options,
//            *options.sift_matching,
//            *options.two_view_geometry,
//            *options.database_path);
//    }
//    else if (match_type == "raw" || match_type == "inliers") {
//        colmap::FeaturePairsMatchingOptions matcher_options;
//        matcher_options.match_list_path = match_list_path;
//        matcher_options.verify_matches = match_type == "raw";
//        matcher = CreateFeaturePairsFeatureMatcher(matcher_options,
//            *options.sift_matching,
//            *options.two_view_geometry,
//            *options.database_path);
//    }
//    else {
//        LOG(ERROR) << "Invalid `match_type`";
//        return EXIT_FAILURE;
//    }
//
//    matcher->Start();
//    matcher->Wait();
//
//    return EXIT_SUCCESS;
//}
//
//int RunSequentialMatcher(int argc, char** argv) {
//    colmap::OptionManager options;
//    options.AddDatabaseOptions();
//    options.AddSequentialMatchingOptions();
//    options.Parse(argc, argv);
//
//    if (!VerifySiftGPUParams(options.sift_matching->use_gpu)) {
//        return EXIT_FAILURE;
//    }
//
//    auto matcher = colmap::CreateSequentialFeatureMatcher(*options.sequential_matching,
//        *options.sift_matching,
//        *options.two_view_geometry,
//        *options.database_path);
//
//    matcher->Start();
//    matcher->Wait();
//
//    return EXIT_SUCCESS;
//}
//
//int RunSpatialMatcher(int argc, char** argv) {
//    colmap::OptionManager options;
//    options.AddDatabaseOptions();
//    options.AddSpatialMatchingOptions();
//    options.Parse(argc, argv);
//
//    if (!VerifySiftGPUParams(options.sift_matching->use_gpu)) {
//        return EXIT_FAILURE;
//    }
//
//    auto matcher = colmap::CreateSpatialFeatureMatcher(*options.spatial_matching,
//        *options.sift_matching,
//        *options.two_view_geometry,
//        *options.database_path);
//
//    matcher->Start();
//    matcher->Wait();
//    
//    return EXIT_SUCCESS;
//}
//
//int RunTransitiveMatcher(int argc, char** argv) {
//    colmap::OptionManager options;
//    options.AddDatabaseOptions();
//    options.AddTransitiveMatchingOptions();
//    options.Parse(argc, argv);
//
//    if (!VerifySiftGPUParams(options.sift_matching->use_gpu)) {
//        return EXIT_FAILURE;
//    }
//
//    auto matcher = colmap::CreateTransitiveFeatureMatcher(*options.transitive_matching,
//        *options.sift_matching,
//        *options.two_view_geometry,
//        *options.database_path);
//
//    matcher->Start();
//    matcher->Wait();
//
//    return EXIT_SUCCESS;
//}
//
//int RunVocabTreeMatcher(int argc, char** argv) {
//    colmap::OptionManager options;
//    options.AddDatabaseOptions();
//    options.AddVocabTreeMatchingOptions();
//    options.Parse(argc, argv);
//
//    if (!VerifySiftGPUParams(options.sift_matching->use_gpu)) {
//        return EXIT_FAILURE;
//    }
//
//    auto matcher = colmap::CreateVocabTreeFeatureMatcher(*options.vocab_tree_matching,
//        *options.sift_matching,
//        *options.two_view_geometry,
//        *options.database_path);
//
//    matcher->Start();
//    matcher->Wait();
//    
//    return EXIT_SUCCESS;
//}
//
MM_NS_E