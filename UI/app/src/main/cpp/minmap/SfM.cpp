#include "SfM.hpp"

#include <controllers/option_manager.h>
#include <estimators/similarity_transform.h>
#include <sfm/observation_manager.h>
#include <util/file.h>

#define MM_ANDROID_LOG_TAG "MINMAP"

MM_NS_B

static std::pair<std::vector<colmap::image_t>, std::vector<Eigen::Vector3d>>
    ExtractExistingImages(const colmap::Reconstruction& reconstruction
) {
    std::vector<colmap::image_t> fixed_image_ids = reconstruction.RegImageIds();
    std::vector<Eigen::Vector3d> orig_fixed_image_positions;
    orig_fixed_image_positions.reserve(fixed_image_ids.size());
    for (const colmap::image_t image_id : fixed_image_ids) {
        orig_fixed_image_positions.push_back(
            reconstruction.Image(image_id).ProjectionCenter());
    }
    return { std::move(fixed_image_ids), std::move(orig_fixed_image_positions) };
}

static void UpdateDatabasePosePriorsCovariance(
    const std::string& database_path,
    const Eigen::Matrix3d& covariance
) {
    colmap::Database database(database_path);
    colmap::DatabaseTransaction database_transaction(&database);

    LOG(MM_INFO)
        << "Setting up database pose priors with the same covariance matrix: \n"
        << covariance << '\n';

    for (const auto& image : database.ReadAllImages()) {
        if (database.ExistsPosePrior(image.ImageId())) {
            colmap::PosePrior prior = database.ReadPosePrior(image.ImageId());
            prior.position_covariance = covariance;
            database.UpdatePosePrior(image.ImageId(), prior);
        }
    }
}

int RunMapper(
    const std::filesystem::path& database_path,
    const std::filesystem::path& image_path,
    const std::filesystem::path& output_path,
    const std::string& input_path,
    const std::string& image_list_path,
    bool fix_existing_frames
) {
    LOG(MM_DEBUG) << "Creating SFM options";
    colmap::OptionManager options(false);
    *options.database_path = database_path.string();
    *options.image_path = image_path.string();
    options.AddDatabaseOptions();
    options.AddImageOptions();
    options.AddMapperOptions();

    // Optional image list
    if (!image_list_path.empty()) {
        options.mapper->image_names = colmap::ReadTextFileLines(image_list_path);
    }

    options.mapper->fix_existing_frames = fix_existing_frames;

    // Android-specific optimizations for mobile devices
    // Reduce computational load during pose estimation to prevent alignment issues
    options.mapper->min_focal_length_ratio = 0.1;
    options.mapper->max_focal_length_ratio = 10.0;

    // Ensure output directory exists
    if (!colmap::ExistsDir(output_path.string())) {
        LOG(MM_ERROR) << "`output_path` is not a directory.";
        return EXIT_FAILURE;
    }

    // Reconstruction manager
    LOG(MM_DEBUG) << "Creating reconstruction manager";
    auto reconstruction_manager = std::make_shared<colmap::ReconstructionManager>();

    std::vector<Eigen::Vector3d> orig_fixed_image_positions;
    std::vector<colmap::image_t> fixed_image_ids;

    if (!input_path.empty()) {
        if (!colmap::ExistsDir(input_path)) {
            LOG(MM_ERROR) << "`input_path` is not a directory.";
            return EXIT_FAILURE;
        }
        LOG(MM_DEBUG) << "Reading the existing reconstruction";
        reconstruction_manager->Read(input_path);

        if (fix_existing_frames && reconstruction_manager->Size() > 0) {
            std::tie(fixed_image_ids, orig_fixed_image_positions) =
                ExtractExistingImages(*reconstruction_manager->Get(0));
        }
    }

    // Run incremental mapper
    colmap::IncrementalPipeline mapper(options.mapper,
        *options.image_path,
        *options.database_path,
        reconstruction_manager);

    // Callback for writing intermediate reconstructions
    size_t prev_num_reconstructions = 0;
    if (input_path.empty()) {
        mapper.AddCallback(colmap::IncrementalPipeline::LAST_IMAGE_REG_CALLBACK, [&]() {
            if (reconstruction_manager->Size() > prev_num_reconstructions) {
                const std::string reconstruction_dir =
                    colmap::JoinPaths(output_path, std::to_string(prev_num_reconstructions));
                colmap::CreateDirIfNotExists(reconstruction_dir);
                reconstruction_manager->Get(prev_num_reconstructions)
                    ->Write(reconstruction_dir);
                options.Write(colmap::JoinPaths(reconstruction_dir, "project.ini"));
                prev_num_reconstructions = reconstruction_manager->Size();
            }
            });
    }

    mapper.Run();

    if (reconstruction_manager->Size() == 0) {
        LOG(MM_ERROR) << "Failed to create sparse model";
        return EXIT_FAILURE;
    }

    // Write final reconstruction if continuing from existing
    if (!input_path.empty()) {
        auto reconstruction = reconstruction_manager->Get(0);

        if (fix_existing_frames) {
            if (fixed_image_ids.size() >= 3) {
                std::vector<Eigen::Vector3d> new_fixed_image_positions;
                new_fixed_image_positions.reserve(fixed_image_ids.size());
                for (colmap::image_t image_id : fixed_image_ids) {
                    new_fixed_image_positions.push_back(
                        reconstruction->Image(image_id).ProjectionCenter());
                }
                colmap::Sim3d orig_from_new;
                if (colmap::EstimateSim3d(new_fixed_image_positions,
                    orig_fixed_image_positions,
                    orig_from_new)) {
                    reconstruction->Transform(orig_from_new);
                }
                else {
                    LOG(MM_WARNING) << "Failed to transform reconstruction to input frame.";
                }
            }
            else {
                LOG(MM_WARNING) << "Too few images to transform reconstruction.";
            }
        }

        reconstruction->Write(output_path.string());
    }

    return EXIT_SUCCESS;
}

MM_NS_E