#include "Feature.hpp"

#include <colmap/controllers/feature_extraction.h>
#include <colmap/controllers/feature_matching.h>
#include <colmap/controllers/image_reader.h>
#include <colmap/controllers/option_manager.h>
#include <colmap/feature/sift.h>
#include <colmap/feature/utils.h>
#include <colmap/sensor/models.h>
#include <colmap/util/file.h>
#include <colmap/util/misc.h>
#include <colmap/util/opengl_utils.h>
#include <colmap/util/threading.h>

MM_NS_B

bool VerifyCameraParams(const std::string& camera_model,
    const std::string& params) {
    if (!colmap::ExistsCameraModelWithName(camera_model)) {
        //LOG(ERROR) << "Camera model does not exist";
        return false;
    }

    const std::vector<double> camera_params = colmap::CSVToVector<double>(params);
    const colmap::CameraModelId camera_model_id = colmap::CameraModelNameToId(camera_model);

    if (camera_params.size() > 0 &&
        !CameraModelVerifyParams(camera_model_id, camera_params)) {
        //LOG(ERROR) << "Invalid camera parameters";
        return false;
    }
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

int RunFeatureExtractor(int argc, char** argv) {
    std::string image_list_path;
    int camera_mode = -1;
    std::string descriptor_normalization = "l1_root";

    colmap::OptionManager options;
    options.AddDatabaseOptions();
    options.AddImageOptions();
    options.AddDefaultOption("camera_mode", &camera_mode);
    options.AddDefaultOption("image_list_path", &image_list_path);
    options.AddDefaultOption("descriptor_normalization",
        &descriptor_normalization,
        "{'l1_root', 'l2'}");
    options.AddFeatureExtractionOptions();
    options.Parse(argc, argv);

    colmap::ImageReaderOptions reader_options = *options.image_reader;
    reader_options.image_path = *options.image_path;
    reader_options.as_rgb = options.feature_extraction->RequiresRGB();

    if (camera_mode >= 0) {
        UpdateImageReaderOptionsFromCameraMode(reader_options,
            (CameraMode)camera_mode);
    }

    colmap::StringToUpper(&descriptor_normalization);
    options.feature_extraction->sift->normalization =
        colmap::SiftExtractionOptions::NormalizationFromString(descriptor_normalization);

    if (!image_list_path.empty()) {
        reader_options.image_names = colmap::ReadTextFileLines(image_list_path);
        if (reader_options.image_names.empty()) {
            return EXIT_SUCCESS;
        }
    }

    if (!colmap::ExistsCameraModelWithName(reader_options.camera_model)) {
        //LOG(ERROR) << "Camera model does not exist";
    }

    if (!VerifyCameraParams(reader_options.camera_model,
        reader_options.camera_params)) {
        return EXIT_FAILURE;
    }

    //std::unique_ptr<QApplication> app;
    //if (options.feature_extraction->use_gpu && kUseOpenGL) {
    //    app.reset(new QApplication(argc, argv));
    //}

    auto feature_extractor = colmap::CreateFeatureExtractorController(
        *options.database_path, reader_options, *options.feature_extraction);

    if (options.feature_extraction->use_gpu && colmap::kUseOpenGL) {
        colmap::RunThreadWithOpenGLContext(feature_extractor.get());
    }
    else {
        feature_extractor->Start();
        feature_extractor->Wait();
    }

    return EXIT_SUCCESS;
}

int RunFeatureImporter(int argc, char** argv) {
    std::string import_path;
    std::string image_list_path;
    int camera_mode = -1;

    colmap::OptionManager options;
    options.AddDatabaseOptions();
    options.AddImageOptions();
    options.AddDefaultOption("camera_mode", &camera_mode);
    options.AddRequiredOption("import_path", &import_path);
    options.AddDefaultOption("image_list_path", &image_list_path);
    options.AddFeatureExtractionOptions();
    options.Parse(argc, argv);

    colmap::ImageReaderOptions reader_options = *options.image_reader;
    reader_options.image_path = *options.image_path;

    if (camera_mode >= 0) {
        UpdateImageReaderOptionsFromCameraMode(reader_options,
            (CameraMode)camera_mode);
    }

    if (!image_list_path.empty()) {
        reader_options.image_names = colmap::ReadTextFileLines(image_list_path);
        if (reader_options.image_names.empty()) {
            return EXIT_SUCCESS;
        }
    }

    if (!VerifyCameraParams(reader_options.camera_model,
        reader_options.camera_params)) {
        return EXIT_FAILURE;
    }

    auto feature_importer = CreateFeatureImporterController(
        *options.database_path, reader_options, import_path);
    feature_importer->Start();
    feature_importer->Wait();

    return EXIT_SUCCESS;
}

int RunExhaustiveMatcher(int argc, char** argv) {
    colmap::OptionManager options;
    options.AddDatabaseOptions();
    options.AddExhaustivePairingOptions();
    options.Parse(argc, argv);

    auto matcher = colmap::CreateExhaustiveFeatureMatcher(*options.exhaustive_pairing,
        *options.feature_matching,
        *options.two_view_geometry,
        *options.database_path);

    if (options.feature_matching->use_gpu && kUseOpenGL) {
        colmap::RunThreadWithOpenGLContext(matcher.get());
    }
    else {
        matcher->Start();
        matcher->Wait();
    }

    return EXIT_SUCCESS;
}

int RunMatchesImporter(int argc, char** argv) {
    std::string match_list_path;
    std::string match_type = "pairs";

    colmap::OptionManager options;
    options.AddDatabaseOptions();
    options.AddRequiredOption("match_list_path", &match_list_path);
    options.AddDefaultOption(
        "match_type", &match_type, "{'pairs', 'raw', 'inliers'}");
    options.AddFeatureMatchingOptions();
    options.AddTwoViewGeometryOptions();
    options.Parse(argc, argv);

    std::unique_ptr<colmap::Thread> matcher;
    if (match_type == "pairs") {
        colmap::ImportedPairingOptions pairing_options;
        pairing_options.match_list_path = match_list_path;
        matcher = colmap::CreateImagePairsFeatureMatcher(pairing_options,
            *options.feature_matching,
            *options.two_view_geometry,
            *options.database_path);
    }
    else if (match_type == "raw" || match_type == "inliers") {
        colmap::FeaturePairsMatchingOptions pairing_options;
        pairing_options.match_list_path = match_list_path;
        pairing_options.verify_matches = match_type == "raw";
        matcher = colmap::CreateFeaturePairsFeatureMatcher(pairing_options,
            *options.feature_matching,
            *options.two_view_geometry,
            *options.database_path);
    }
    else {
        //LOG(ERROR) << "Invalid `match_type`";
        return EXIT_FAILURE;
    }

    if (options.feature_matching->use_gpu && kUseOpenGL) {
        colmap::RunThreadWithOpenGLContext(matcher.get());
    }
    else {
        matcher->Start();
        matcher->Wait();
    }

    return EXIT_SUCCESS;
}

int RunSequentialMatcher(int argc, char** argv) {
    colmap::OptionManager options;
    options.AddDatabaseOptions();
    options.AddSequentialPairingOptions();
    options.Parse(argc, argv);

    auto matcher = colmap::CreateSequentialFeatureMatcher(*options.sequential_pairing,
        *options.feature_matching,
        *options.two_view_geometry,
        *options.database_path);

    if (options.feature_matching->use_gpu && kUseOpenGL) {
        colmap::RunThreadWithOpenGLContext(matcher.get());
    }
    else {
        matcher->Start();
        matcher->Wait();
    }

    return EXIT_SUCCESS;
}

int RunSpatialMatcher(int argc, char** argv) {
    colmap::OptionManager options;
    options.AddDatabaseOptions();
    options.AddSpatialPairingOptions();
    if (!options.Parse(argc, argv)) {
        return EXIT_FAILURE;
    }

    auto matcher = colmap::CreateSpatialFeatureMatcher(*options.spatial_pairing,
        *options.feature_matching,
        *options.two_view_geometry,
        *options.database_path);

    if (options.feature_matching->use_gpu && kUseOpenGL) {
        colmap::RunThreadWithOpenGLContext(matcher.get());
    }
    else {
        matcher->Start();
        matcher->Wait();
    }

    return EXIT_SUCCESS;
}

int RunTransitiveMatcher(int argc, char** argv) {
    colmap::OptionManager options;
    options.AddDatabaseOptions();
    options.AddTransitivePairingOptions();
    if (!options.Parse(argc, argv)) {
        return EXIT_FAILURE;
    }

    std::unique_ptr<QApplication> app;
    if (options.feature_matching->use_gpu && kUseOpenGL) {
        app.reset(new QApplication(argc, argv));
    }

    auto matcher = colmap::CreateTransitiveFeatureMatcher(*options.transitive_pairing,
        *options.feature_matching,
        *options.two_view_geometry,
        *options.database_path);

    if (options.feature_matching->use_gpu && kUseOpenGL) {
        colmap::RunThreadWithOpenGLContext(matcher.get());
    }
    else {
        matcher->Start();
        matcher->Wait();
    }

    return EXIT_SUCCESS;
}

int RunVocabTreeMatcher(int argc, char** argv) {
    colmap::OptionManager options;
    options.AddDatabaseOptions();
    options.AddVocabTreePairingOptions();
    options.Parse(argc, argv);

    auto matcher = colmap::CreateVocabTreeFeatureMatcher(*options.vocab_tree_pairing,
        *options.feature_matching,
        *options.two_view_geometry,
        *options.database_path);

    if (options.feature_matching->use_gpu && kUseOpenGL) {
        colmap::RunThreadWithOpenGLContext(matcher.get());
    }
    else {
        matcher->Start();
        matcher->Wait();
    }

    return EXIT_SUCCESS;
}

MM_NS_E