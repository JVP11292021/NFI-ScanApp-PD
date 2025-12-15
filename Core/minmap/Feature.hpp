#ifndef MINMAP_FEATURE_H
#define MINMAP_FEATURE_H

#include "minmap_defs.hpp"

#include <colmap/feature/pairing.h>
#include <colmap/controllers/feature_matching.h>
#include <colmap/controllers/image_reader.h>

MM_NS_B

// Note: When using AUTO mode a camera model will be uniquely identified by the
// following 5 parameters from EXIF tags:
// 1. Camera Make
// 2. Camera Model
// 3. Focal Length
// 4. Image Width
// 5. Image Height
//
// If any of the tags is missing then a camera model is considered invalid and a
// new camera is created similar to the PER_IMAGE mode.
//
// If these considered fields are not sufficient to uniquely identify a camera
// then using the AUTO mode will lead to incorrect setup for the cameras, e.g.
// the same camera is used with same focal length but different principal point
// between captures. In these cases it is recommended to either use the
// PER_FOLDER or PER_IMAGE settings.
enum class CameraMode { AUTO = 0, SINGLE = 1, PER_FOLDER = 2, PER_IMAGE = 3 };

void UpdateImageReaderOptionsFromCameraMode(colmap::ImageReaderOptions& options, CameraMode mode);

bool VerifySiftGPUParams(bool use_gpu);

bool VerifyCameraParams(const std::string& camera_model,
    const std::string& params);

int RunFeatureExtractor(int argc, char** argv);
int RunFeatureImporter(int argc, char** argv);
int RunExhaustiveMatcher(int argc, char** argv);
int RunMatchesImporter(int argc, char** argv);
int RunSequentialMatcher(int argc, char** argv);
int RunSpatialMatcher(int argc, char** argv);
int RunTransitiveMatcher(int argc, char** argv);
int RunVocabTreeMatcher(int argc, char** argv);

MM_NS_E

#endif // MINMAP_FEATURE_H