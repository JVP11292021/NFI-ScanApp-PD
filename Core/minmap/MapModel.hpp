#ifndef MINMAP_MODEL_H
#define MINMAP_MODEL_H

#include "minmap_defs.hpp"

#include <filesystem>
#include <string>

//#include "colmap/estimators/alignment.h"
//#include "colmap/geometry/sim3.h"
//#include "colmap/scene/reconstruction.h"

MM_NS_B

int RunModelConverter(
    const std::filesystem::path& input_path,
    const std::filesystem::path& output_path,
    const std::string& output_type,
    bool skip_distortion = false);

//bool CompareModels(const colmap::Reconstruction& reconstruction1,
//    const colmap::Reconstruction& reconstruction2,
//    const std::string& alignment_error,
//    double min_inlier_observations,
//    double max_reproj_error,
//    double max_proj_center_error,
//    std::vector<colmap::ImageAlignmentError>& errors,
//    colmap::Sim3d& rec2_from_rec1);

//int RunModelAligner(int argc, char** argv);
//int RunModelAnalyzer(int argc, char** argv);
//int RunModelComparer(int argc, char** argv);
//int RunModelCropper(int argc, char** argv);
//int RunModelMerger(int argc, char** argv);
//int RunModelOrientationAligner(int argc, char** argv);
//int RunModelSplitter(int argc, char** argv);
//int RunModelTransformer(int argc, char** argv);


MM_NS_E

#endif // MINMAP_MODEL_H