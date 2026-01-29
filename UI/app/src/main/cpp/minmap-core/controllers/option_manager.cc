#include "option_manager.h"

#include "feature_extraction.h"
#include "image_reader.h"
#include "incremental_pipeline.h"

#include "../util/logging.h"
#include "../estimators/two_view_geometry.h"
#include "../feature/pairing.h"
#include "../feature/sift.h"
#include "../math/random.h"
#include "../util/file.h"

namespace colmap {

OptionManager::OptionManager(bool add_project_options) {
  project_path = std::make_shared<std::string>();
  database_path = std::make_shared<std::string>();
  image_path = std::make_shared<std::string>();

  image_reader = std::make_shared<ImageReaderOptions>();
  sift_extraction = std::make_shared<SiftExtractionOptions>();
  sift_matching = std::make_shared<SiftMatchingOptions>();
  two_view_geometry = std::make_shared<TwoViewGeometryOptions>();
  exhaustive_matching = std::make_shared<ExhaustiveMatchingOptions>();
  transitive_matching = std::make_shared<TransitiveMatchingOptions>();
  image_pairs_matching = std::make_shared<ImagePairsMatchingOptions>();
  mapper = std::make_shared<IncrementalPipelineOptions>();

  Reset();

  AddRandomOptions();
  AddLogOptions();
}

void OptionManager::ModifyForIndividualData() {
  mapper->min_focal_length_ratio = 0.1;
  mapper->max_focal_length_ratio = 10;
  mapper->max_extra_param = std::numeric_limits<double>::max();
}

void OptionManager::ModifyForVideoData() {
  const bool kResetPaths = false;
  ResetOptions(kResetPaths);
  mapper->mapper.init_min_tri_angle /= 2;
  mapper->ba_global_frames_ratio = 1.4;
  mapper->ba_global_points_ratio = 1.4;
  mapper->min_focal_length_ratio = 0.1;
  mapper->max_focal_length_ratio = 10;
  mapper->max_extra_param = std::numeric_limits<double>::max();
}

void OptionManager::ModifyForLowQuality() {
    sift_extraction->max_image_size = 1000;
    sift_extraction->max_num_features = 2048;
    sift_extraction->use_gpu = false; // Mobile
    sift_extraction->num_threads = 3;
    sift_matching->use_gpu = false; // Mobile
    sift_matching->num_threads = 3;
    mapper->ba_local_max_num_iterations /= 2;
    mapper->ba_global_max_num_iterations /= 2;
    mapper->ba_global_frames_ratio *= 1.2;
    mapper->ba_global_points_ratio *= 1.2;
    mapper->ba_global_max_refinements = 3;  // Increased from 2 for more robust refinement
    mapper->num_threads = 3;

    // Relax initialization and filtering constraints for low quality data
    // For mobile scanning with close range objects, we need much more lenient angle thresholds
    mapper->mapper.init_min_tri_angle = 1.0;  // Aggressively reduce to allow small baselines
    mapper->mapper.init_min_num_inliers = std::max(10, static_cast<int>(mapper->mapper.init_min_num_inliers / 4));
    mapper->mapper.abs_pose_min_num_inliers = std::max(4, static_cast<int>(mapper->mapper.abs_pose_min_num_inliers / 2));
    mapper->mapper.filter_max_reproj_error *= 2.0;  // More lenient point filtering for jittery mobile data
    mapper->mapper.filter_min_tri_angle = 0.1;  // Very lenient - allow almost any triangulation
    mapper->mapper.local_ba_min_tri_angle = 0.1;  // Relax local BA angle constraint
}

void OptionManager::ModifyForMediumQuality() {
    sift_extraction->max_image_size = 1600;
    sift_extraction->max_num_features = 4096;
    sift_extraction->use_gpu = false; // Mobile
    sift_extraction->num_threads = 4;
    sift_matching->use_gpu = false; // Mobile
    sift_matching->num_threads = 4;
    mapper->ba_local_max_num_iterations /= 1.5;
    mapper->ba_global_max_num_iterations /= 1.5;
    mapper->ba_global_frames_ratio *= 1.1;
    mapper->ba_global_points_ratio *= 1.1;
    mapper->ba_global_max_refinements = 2;
    mapper->num_threads = 4;

    // Relax initialization and filtering constraints for low quality data
    // For mobile scanning with close range objects, we need much more lenient angle thresholds
    mapper->mapper.init_min_tri_angle /= 2;  // Further reduce minimum triangulation angle
    mapper->mapper.init_min_num_inliers = std::max(10, static_cast<int>(mapper->mapper.init_min_num_inliers / 3));
    mapper->mapper.filter_max_reproj_error *= 1.5;  // More lenient point filtering
    mapper->mapper.filter_min_tri_angle /= 2;  // Allow points with smaller triangulation angles
}

void OptionManager::ModifyForHighQuality() {
    sift_extraction->estimate_affine_shape = true;
    sift_extraction->max_image_size = 2400;
    sift_extraction->max_num_features = 8192;
    sift_extraction->use_gpu = false; // Mobile
    sift_extraction->num_threads = 5;
    sift_matching->guided_matching = true;
    sift_matching->use_gpu = false; // Mobile
    sift_matching->num_threads = 5;
    mapper->ba_local_max_num_iterations = 30;
    mapper->ba_local_max_refinements = 3;
    mapper->ba_global_max_num_iterations = 75;
    mapper->num_threads = 5;

    // Relax initialization and filtering constraints for low quality data
    // For mobile scanning with close range objects, we need much more lenient angle thresholds
    mapper->mapper.init_min_tri_angle /= 2;  // Further reduce minimum triangulation angle
    mapper->mapper.init_min_num_inliers = std::max(10, static_cast<int>(mapper->mapper.init_min_num_inliers / 3));
    mapper->mapper.filter_max_reproj_error *= 1.5;  // More lenient point filtering
    mapper->mapper.filter_min_tri_angle /= 2;  // Allow points with smaller triangulation angles
}

void OptionManager::ModifyForExtremeQuality() {
    // Most of the options are set to extreme quality by default.
    sift_extraction->estimate_affine_shape = true;
    sift_extraction->domain_size_pooling = true;
    sift_extraction->use_gpu = false; // Mobile
    sift_matching->guided_matching = true;
    sift_matching->use_gpu = false; // Mobile
    mapper->ba_local_max_num_iterations = 40;
    mapper->ba_local_max_refinements = 3;
    mapper->ba_global_max_num_iterations = 100;

    // Relax initialization and filtering constraints for low quality data
    // For mobile scanning with close range objects, we need much more lenient angle thresholds
    mapper->mapper.init_min_tri_angle /= 2;  // Further reduce minimum triangulation angle
    mapper->mapper.init_min_num_inliers = std::max(10, static_cast<int>(mapper->mapper.init_min_num_inliers / 3));
    mapper->mapper.filter_max_reproj_error *= 1.5;  // More lenient point filtering
    mapper->mapper.filter_min_tri_angle /= 2;  // Allow points with smaller triangulation angles
}

void OptionManager::AddAllOptions() {
  AddLogOptions();
  AddRandomOptions();
  AddDatabaseOptions();
  AddImageOptions();
  AddExtractionOptions();
  AddMatchingOptions();
  AddExhaustiveMatchingOptions();
  AddTransitiveMatchingOptions();
  AddImagePairsMatchingOptions();
  AddMapperOptions();
}

void OptionManager::AddLogOptions() {
  if (added_log_options_) {
    return;
  }
  added_log_options_ = true;
}

void OptionManager::AddRandomOptions() {
  if (added_random_options_) {
    return;
  }
  added_random_options_ = true;

  Register("random_seed", &kDefaultPRNGSeed);
}

void OptionManager::AddDatabaseOptions() {
  if (added_database_options_) {
    return;
  }
  added_database_options_ = true;

  Register("database_path", database_path.get());
}

void OptionManager::AddImageOptions() {
  if (added_image_options_) {
    return;
  }
  added_image_options_ = true;

  Register("image_path", image_path.get());
}

void OptionManager::AddExtractionOptions() {
  if (added_extraction_options_) {
    return;
  }
  added_extraction_options_ = true;

  Register("ImageReader.mask_path",
                              &image_reader->mask_path);
  Register("ImageReader.camera_model",
                              &image_reader->camera_model);
  Register("ImageReader.single_camera",
                              &image_reader->single_camera);
  Register("ImageReader.single_camera_per_folder",
                              &image_reader->single_camera_per_folder);
  Register("ImageReader.single_camera_per_image",
                              &image_reader->single_camera_per_image);
  Register("ImageReader.existing_camera_id",
                              &image_reader->existing_camera_id);
  Register("ImageReader.camera_params",
                              &image_reader->camera_params);
  Register("ImageReader.default_focal_length_factor",
                              &image_reader->default_focal_length_factor);
  Register("ImageReader.camera_mask_path",
                              &image_reader->camera_mask_path);

  Register("SiftExtraction.num_threads",
                              &sift_extraction->num_threads);
  Register("SiftExtraction.use_gpu",
                              &sift_extraction->use_gpu);
  Register("SiftExtraction.gpu_index",
                              &sift_extraction->gpu_index);
  Register("SiftExtraction.max_image_size",
                              &sift_extraction->max_image_size);
  Register("SiftExtraction.max_num_features",
                              &sift_extraction->max_num_features);
  Register("SiftExtraction.first_octave",
                              &sift_extraction->first_octave);
  Register("SiftExtraction.num_octaves",
                              &sift_extraction->num_octaves);
  Register("SiftExtraction.octave_resolution",
                              &sift_extraction->octave_resolution);
  Register("SiftExtraction.peak_threshold",
                              &sift_extraction->peak_threshold);
  Register("SiftExtraction.edge_threshold",
                              &sift_extraction->edge_threshold);
  Register("SiftExtraction.estimate_affine_shape",
                              &sift_extraction->estimate_affine_shape);
  Register("SiftExtraction.max_num_orientations",
                              &sift_extraction->max_num_orientations);
  Register("SiftExtraction.upright",
                              &sift_extraction->upright);
  Register("SiftExtraction.domain_size_pooling",
                              &sift_extraction->domain_size_pooling);
  Register("SiftExtraction.dsp_min_scale",
                              &sift_extraction->dsp_min_scale);
  Register("SiftExtraction.dsp_max_scale",
                              &sift_extraction->dsp_max_scale);
  Register("SiftExtraction.dsp_num_scales",
                              &sift_extraction->dsp_num_scales);
}

void OptionManager::AddMatchingOptions() {
  if (added_match_options_) {
    return;
  }
  added_match_options_ = true;

  Register("SiftMatching.num_threads",
                              &sift_matching->num_threads);
  Register("SiftMatching.use_gpu", &sift_matching->use_gpu);
  Register("SiftMatching.gpu_index",
                              &sift_matching->gpu_index);
  Register("SiftMatching.max_ratio",
                              &sift_matching->max_ratio);
  Register("SiftMatching.max_distance",
                              &sift_matching->max_distance);
  Register("SiftMatching.cross_check",
                              &sift_matching->cross_check);
  Register("SiftMatching.guided_matching",
                              &sift_matching->guided_matching);
  Register("SiftMatching.max_num_matches",
                              &sift_matching->max_num_matches);
  Register("SiftMatching.cpu_brute_force_matcher",
                              &sift_matching->cpu_brute_force_matcher);
  Register("TwoViewGeometry.min_num_inliers",
                              &two_view_geometry->min_num_inliers);
  Register("TwoViewGeometry.multiple_models",
                              &two_view_geometry->multiple_models);
  Register("TwoViewGeometry.compute_relative_pose",
                              &two_view_geometry->compute_relative_pose);
  Register("TwoViewGeometry.max_error",
                              &two_view_geometry->ransac_options.max_error);
  Register("TwoViewGeometry.confidence",
                              &two_view_geometry->ransac_options.confidence);
  Register(
      "TwoViewGeometry.max_num_trials",
      &two_view_geometry->ransac_options.max_num_trials);
  Register(
      "TwoViewGeometry.min_inlier_ratio",
      &two_view_geometry->ransac_options.min_inlier_ratio);
}

void OptionManager::AddExhaustiveMatchingOptions() {
  if (added_exhaustive_match_options_) {
    return;
  }
  added_exhaustive_match_options_ = true;

  AddMatchingOptions();

  Register("ExhaustiveMatching.block_size",
                              &exhaustive_matching->block_size);
}


void OptionManager::AddTransitiveMatchingOptions() {
  if (added_transitive_match_options_) {
    return;
  }
  added_transitive_match_options_ = true;

  AddMatchingOptions();

  Register("TransitiveMatching.batch_size",
                              &transitive_matching->batch_size);
  Register("TransitiveMatching.num_iterations",
                              &transitive_matching->num_iterations);
}

void OptionManager::AddImagePairsMatchingOptions() {
  if (added_image_pairs_match_options_) {
    return;
  }
  added_image_pairs_match_options_ = true;

  AddMatchingOptions();

  Register("ImagePairsMatching.block_size",
                              &image_pairs_matching->block_size);
}

void OptionManager::AddMapperOptions() {
  if (added_mapper_options_) {
    return;
  }
  added_mapper_options_ = true;

  Register("Mapper.min_num_matches",
                              &mapper->min_num_matches);
  Register("Mapper.ignore_watermarks",
                              &mapper->ignore_watermarks);
  Register("Mapper.multiple_models",
                              &mapper->multiple_models);
  Register("Mapper.max_num_models", &mapper->max_num_models);
  Register("Mapper.max_model_overlap",
                              &mapper->max_model_overlap);
  Register("Mapper.min_model_size", &mapper->min_model_size);
  Register("Mapper.init_image_id1", &mapper->init_image_id1);
  Register("Mapper.init_image_id2", &mapper->init_image_id2);
  Register("Mapper.init_num_trials",
                              &mapper->init_num_trials);
  Register("Mapper.extract_colors", &mapper->extract_colors);
  Register("Mapper.num_threads", &mapper->num_threads);
  Register("Mapper.min_focal_length_ratio",
                              &mapper->min_focal_length_ratio);
  Register("Mapper.max_focal_length_ratio",
                              &mapper->max_focal_length_ratio);
  Register("Mapper.max_extra_param",
                              &mapper->max_extra_param);
  Register("Mapper.ba_refine_focal_length",
                              &mapper->ba_refine_focal_length);
  Register("Mapper.ba_refine_principal_point",
                              &mapper->ba_refine_principal_point);
  Register("Mapper.ba_refine_extra_params",
                              &mapper->ba_refine_extra_params);
  Register("Mapper.ba_refine_sensor_from_rig",
                              &mapper->ba_refine_sensor_from_rig);
  Register("Mapper.ba_local_num_images",
                              &mapper->ba_local_num_images);
  Register("Mapper.ba_local_function_tolerance",
                              &mapper->ba_local_function_tolerance);
  Register("Mapper.ba_local_max_num_iterations",
                              &mapper->ba_local_max_num_iterations);
  Register("Mapper.ba_global_frames_ratio",
                              &mapper->ba_global_frames_ratio);
  Register("Mapper.ba_global_points_ratio",
                              &mapper->ba_global_points_ratio);
  Register("Mapper.ba_global_frames_freq",
                              &mapper->ba_global_frames_freq);
  Register("Mapper.ba_global_points_freq",
                              &mapper->ba_global_points_freq);
  Register("Mapper.ba_global_function_tolerance",
                              &mapper->ba_global_function_tolerance);
  Register("Mapper.ba_global_max_num_iterations",
                              &mapper->ba_global_max_num_iterations);
  Register("Mapper.ba_global_max_refinements",
                              &mapper->ba_global_max_refinements);
  Register("Mapper.ba_global_max_refinement_change",
                              &mapper->ba_global_max_refinement_change);
  Register("Mapper.ba_local_max_refinements",
                              &mapper->ba_local_max_refinements);
  Register("Mapper.ba_local_max_refinement_change",
                              &mapper->ba_local_max_refinement_change);
  Register("Mapper.ba_use_gpu", &mapper->ba_use_gpu);
  Register("Mapper.ba_gpu_index", &mapper->ba_gpu_index);
  Register(
      "Mapper.ba_min_num_residuals_for_cpu_multi_threading",
      &mapper->ba_min_num_residuals_for_cpu_multi_threading);
  Register("Mapper.snapshot_path", &mapper->snapshot_path);
  Register("Mapper.snapshot_frames_freq",
                              &mapper->snapshot_frames_freq);
  Register("Mapper.fix_existing_frames",
                              &mapper->fix_existing_frames);

  // IncrementalMapper.
  Register("Mapper.init_min_num_inliers",
                              &mapper->mapper.init_min_num_inliers);
  Register("Mapper.init_max_error",
                              &mapper->mapper.init_max_error);
  Register("Mapper.init_max_forward_motion",
                              &mapper->mapper.init_max_forward_motion);
  Register("Mapper.init_min_tri_angle",
                              &mapper->mapper.init_min_tri_angle);
  Register("Mapper.init_max_reg_trials",
                              &mapper->mapper.init_max_reg_trials);
  Register("Mapper.abs_pose_max_error",
                              &mapper->mapper.abs_pose_max_error);
  Register("Mapper.abs_pose_min_num_inliers",
                              &mapper->mapper.abs_pose_min_num_inliers);
  Register("Mapper.abs_pose_min_inlier_ratio",
                              &mapper->mapper.abs_pose_min_inlier_ratio);
  Register("Mapper.filter_max_reproj_error",
                              &mapper->mapper.filter_max_reproj_error);
  Register("Mapper.filter_min_tri_angle",
                              &mapper->mapper.filter_min_tri_angle);
  Register("Mapper.max_reg_trials",
                              &mapper->mapper.max_reg_trials);
  Register("Mapper.local_ba_min_tri_angle",
                              &mapper->mapper.local_ba_min_tri_angle);

  // IncrementalTriangulator.
  Register("Mapper.tri_max_transitivity",
                              &mapper->triangulation.max_transitivity);
  Register("Mapper.tri_create_max_angle_error",
                              &mapper->triangulation.create_max_angle_error);
  Register("Mapper.tri_continue_max_angle_error",
                              &mapper->triangulation.continue_max_angle_error);
  Register("Mapper.tri_merge_max_reproj_error",
                              &mapper->triangulation.merge_max_reproj_error);
  Register("Mapper.tri_complete_max_reproj_error",
                              &mapper->triangulation.complete_max_reproj_error);
  Register("Mapper.tri_complete_max_transitivity",
                              &mapper->triangulation.complete_max_transitivity);
  Register("Mapper.tri_re_max_angle_error",
                              &mapper->triangulation.re_max_angle_error);
  Register("Mapper.tri_re_min_ratio",
                              &mapper->triangulation.re_min_ratio);
  Register("Mapper.tri_re_max_trials",
                              &mapper->triangulation.re_max_trials);
  Register("Mapper.tri_min_angle",
                              &mapper->triangulation.min_angle);
  Register("Mapper.tri_ignore_two_view_tracks",
                              &mapper->triangulation.ignore_two_view_tracks);
}

void OptionManager::Reset() {
  const bool kResetPaths = true;
  ResetOptions(kResetPaths);

  added_log_options_ = false;
  added_random_options_ = false;
  added_database_options_ = false;
  added_image_options_ = false;
  added_extraction_options_ = false;
  added_match_options_ = false;
  added_exhaustive_match_options_ = false;
  added_sequential_match_options_ = false;
  added_vocab_tree_match_options_ = false;
  added_spatial_match_options_ = false;
  added_transitive_match_options_ = false;
  added_image_pairs_match_options_ = false;
  added_ba_options_ = false;
  added_mapper_options_ = false;
  added_patch_match_stereo_options_ = false;
  added_stereo_fusion_options_ = false;
  added_poisson_meshing_options_ = false;
  added_delaunay_meshing_options_ = false;
  added_render_options_ = false;
}

void OptionManager::ResetOptions(const bool reset_paths) {
  if (reset_paths) {
    *project_path = "";
    *database_path = "";
    *image_path = "";
  }
  *image_reader = ImageReaderOptions();
  *sift_extraction = SiftExtractionOptions();
  *sift_matching = SiftMatchingOptions();
  *exhaustive_matching = ExhaustiveMatchingOptions();
  *transitive_matching = TransitiveMatchingOptions();
  *image_pairs_matching = ImagePairsMatchingOptions();
  *mapper = IncrementalPipelineOptions();
}

bool OptionManager::Check() {
  bool success = true;

  if (added_database_options_) {
    const auto database_parent_path = GetParentDir(*database_path);
    success = success && CHECK_OPTION_IMPL(!ExistsDir(*database_path)) &&
              CHECK_OPTION_IMPL(database_parent_path == "" ||
                                ExistsDir(database_parent_path));
  }

  if (added_image_options_)
    success = success && ExistsDir(*image_path);

  if (image_reader) success = success && image_reader->Check();
  if (sift_extraction) success = success && sift_extraction->Check();

  if (sift_matching) success = success && sift_matching->Check();
  if (two_view_geometry) success = success && two_view_geometry->Check();
  if (exhaustive_matching) success = success && exhaustive_matching->Check();
  if (transitive_matching) success = success && transitive_matching->Check();
  if (image_pairs_matching) success = success && image_pairs_matching->Check();

  if (mapper) success = success && mapper->Check();

  return success;
}

bool OptionManager::Read(const std::string& path) {
    std::ifstream f(path);
    if (!f) return false;

    std::string line;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#' || line[0] == '[') continue;
        auto pos = line.find('=');
        if (pos == std::string::npos) continue;

        std::string key = line.substr(0, pos);
        std::string val = line.substr(pos + 1);

        auto it = entries_.find(key);
        if (it != entries_.end())
            it->second.set(val);
    }
    return Check();
}

bool OptionManager::ReRead(const std::string& path) {
  Reset();
  AddAllOptions();
  return Read(path);
}

void OptionManager::Write(const std::string & path) const {
    std::ofstream f(path);
    for (auto& kv : entries_)
        f << kv.first << "=" << kv.second.get() << "\n";
}

}  // namespace colmap
