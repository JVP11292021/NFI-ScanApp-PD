#pragma once

#include "../util/types.h"

#include <PoseLib/alignment.h>
#include <Eigen/Core>

namespace colmap {

// 2D point class corresponds to a feature in an image. It may or may not have a
// corresponding 3D point if it is part of a triangulated track.
struct Point2D {
  // The image coordinates in pixels, starting at upper left corner with 0.
  Eigen::Vector2d xy = Eigen::Vector2d::Zero();

  // The identifier of the 3D point. If the 2D point is not part of a 3D point
  // track the identifier is `kInvalidPoint3DId` and `HasPoint3D() = false`.
  point3D_t point3D_id = kInvalidPoint3DId;

  // Determin whether the 2D point observes a 3D point.
  inline bool HasPoint3D() const;

  inline bool operator==(const Point2D& other) const;
  inline bool operator!=(const Point2D& other) const;
};

std::ostream& operator<<(std::ostream& stream, const Point2D& point2D);

////////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////////

bool Point2D::HasPoint3D() const { return point3D_id != kInvalidPoint3DId; }

bool Point2D::operator==(const Point2D& other) const {
  return xy == other.xy && point3D_id == other.point3D_id;
}

bool Point2D::operator!=(const Point2D& other) const {
  return !(*this == other);
}

}  // namespace colmap
