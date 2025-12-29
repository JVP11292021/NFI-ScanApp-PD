#ifndef MINMAP_SFM_H
#define MINMAP_SFM_H

#include "minmap_defs.hpp"

#include "colmap/controllers/incremental_pipeline.h"
#include "colmap/scene/reconstruction.h"

MM_NS_B

int RunMapper(int argc, char** argv);

//void RunPointTriangulatorImpl(
//    const std::shared_ptr<colmap::Reconstruction>& reconstruction,
//    const std::string& database_path,
//    const std::string& image_path,
//    const std::string& output_path,
//    const colmap::IncrementalPipelineOptions& options,
//    bool clear_points,
//    bool refine_intrinsics);
//
//int RunAutomaticReconstructor(int argc, char** argv);
//int RunBundleAdjuster(int argc, char** argv);
//int RunColorExtractor(int argc, char** argv);
//int RunHierarchicalMapper(int argc, char** argv);
//int RunPosePriorMapper(int argc, char** argv);
//int RunPointFiltering(int argc, char** argv);
//int RunPointTriangulator(int argc, char** argv);
//int RunRigBundleAdjuster(int argc, char** argv);


MM_NS_E

#endif // MINMAP_SFM_H