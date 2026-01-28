#pragma once

#include "reconstruction.h"

#include <iostream>

namespace colmap {

// Note that cameras must be read before images.

void ReadRigsBinary(Reconstruction& reconstruction, std::istream& stream);
void ReadRigsBinary(Reconstruction& reconstruction, const std::string& path);

void ReadCamerasBinary(Reconstruction& reconstruction, std::istream& stream);
void ReadCamerasBinary(Reconstruction& reconstruction, const std::string& path);

void ReadFramesBinary(Reconstruction& reconstruction, std::istream& stream);
void ReadFramesBinary(Reconstruction& reconstruction, const std::string& path);

void ReadImagesBinary(Reconstruction& reconstruction, std::istream& stream);
void ReadImagesBinary(Reconstruction& reconstruction, const std::string& path);

void ReadPoints3DBinary(Reconstruction& reconstruction, std::istream& stream);
void ReadPoints3DBinary(Reconstruction& reconstruction,
                        const std::string& path);

void WriteRigsBinary(const Reconstruction& reconstruction,
                     std::ostream& stream);
void WriteRigsBinary(const Reconstruction& reconstruction,
                     const std::string& path);

void WriteFramesBinary(const Reconstruction& reconstruction,
                       std::ostream& stream);
void WriteFramesBinary(const Reconstruction& reconstruction,
                       const std::string& path);

void WriteCamerasBinary(const Reconstruction& reconstruction,
                        std::ostream& stream);
void WriteCamerasBinary(const Reconstruction& reconstruction,
                        const std::string& path);

void WriteImagesBinary(const Reconstruction& reconstruction,
                       std::ostream& stream);
void WriteImagesBinary(const Reconstruction& reconstruction,
                       const std::string& path);

void WritePoints3DBinary(const Reconstruction& reconstruction,
                         std::ostream& stream);
void WritePoints3DBinary(const Reconstruction& reconstruction,
                         const std::string& path);

}  // namespace colmap
