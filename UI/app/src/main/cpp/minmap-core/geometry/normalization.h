#pragma once

#include <vector>

#include <PoseLib/alignment.h>
#include <Eigen/Core>
#include <Eigen/Geometry>

namespace colmap {

// Computes axis aligned bounding box for coordinates within the given
// percentile range. Computes the centroid as the mean within the box.
std::pair<Eigen::AlignedBox3d, Eigen::Vector3d> ComputeBoundingBoxAndCentroid(
    double min_percentile,
    double max_percentile,
    std::vector<double> coords_x,
    std::vector<double> coords_y,
    std::vector<double> coords_z);

}  // namespace colmap
