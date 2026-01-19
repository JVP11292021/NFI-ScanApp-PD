#include "image.h"

#include "../geometry/pose.h"
#include "../scene/projection.h"

namespace colmap {

Image::Image()
    : image_id_(kInvalidImageId),
      name_(""),
      camera_id_(kInvalidCameraId),
      camera_ptr_(nullptr),
      frame_id_(kInvalidFrameId),
      frame_ptr_(nullptr),
      num_points3D_(0) {}

Image::Image(const Image& other)
    : image_id_(other.ImageId()),
      name_(other.Name()),
      camera_id_(other.CameraId()),
      camera_ptr_(other.HasCameraPtr() ? other.CameraPtr() : nullptr),
      frame_id_(other.FrameId()),
      frame_ptr_(other.HasFramePtr() ? other.FramePtr() : nullptr),
      num_points3D_(other.NumPoints3D()),
      points2D_(other.Points2D()) {}

Image& Image::operator=(const Image& other) {
  if (this != &other) {
    image_id_ = other.ImageId();
    name_ = other.Name();
    camera_id_ = other.CameraId();
    if (other.HasCameraPtr()) {
      camera_ptr_ = other.CameraPtr();
    } else {
      camera_ptr_ = nullptr;
    }
    if (other.HasFramePtr()) {
      frame_ptr_ = other.FramePtr();
    } else {
      frame_ptr_ = nullptr;
    }
    num_points3D_ = other.NumPoints3D();
    points2D_ = other.Points2D();
  }
  return *this;
}

void Image::SetPoints2D(const std::vector<Eigen::Vector2d>& points) {
  points2D_.resize(points.size());
  for (point2D_t point2D_idx = 0; point2D_idx < points.size(); ++point2D_idx) {
    points2D_[point2D_idx].xy = points[point2D_idx];
  }
}

void Image::SetPoints2D(const std::vector<struct Point2D>& points) {
  THROW_CHECK(points2D_.empty());
  points2D_ = points;
  num_points3D_ = 0;
  for (const auto& point2D : points2D_) {
    if (point2D.HasPoint3D()) {
      num_points3D_ += 1;
    }
  }
}

void Image::SetPoint3DForPoint2D(const point2D_t point2D_idx,
                                 const point3D_t point3D_id) {
  THROW_CHECK_NE(point3D_id, kInvalidPoint3DId);
  struct Point2D& point2D = points2D_.at(point2D_idx);
  if (!point2D.HasPoint3D()) {
    num_points3D_ += 1;
  }
  point2D.point3D_id = point3D_id;
}

void Image::ResetPoint3DForPoint2D(const point2D_t point2D_idx) {
  struct Point2D& point2D = points2D_.at(point2D_idx);
  if (point2D.HasPoint3D()) {
    point2D.point3D_id = kInvalidPoint3DId;
    num_points3D_ -= 1;
  }
}

bool Image::HasPoint3D(const point3D_t point3D_id) const {
  return std::find_if(points2D_.begin(),
                      points2D_.end(),
                      [point3D_id](const struct Point2D& point2D) {
                        return point2D.point3D_id == point3D_id;
                      }) != points2D_.end();
}

Eigen::Vector3d Image::ProjectionCenter() const {
  return CamFromWorld().rotation.inverse() * -CamFromWorld().translation;
}

Eigen::Vector3d Image::ViewingDirection() const {
  return CamFromWorld().rotation.toRotationMatrix().row(2);
}

std::optional<Eigen::Vector2d> Image::ProjectPoint(
    const Eigen::Vector3d& point3D) const {
  THROW_CHECK(HasCameraPtr());
  const Eigen::Vector3d point3D_in_cam = CamFromWorld() * point3D;
  return camera_ptr_->ImgFromCam(point3D_in_cam);
}

std::ostream& operator<<(std::ostream& stream, const Image& image) {
  stream << "Image(image_id="
         << (image.ImageId() != kInvalidImageId
                 ? std::to_string(image.ImageId())
                 : "Invalid");
  if (!image.HasCameraPtr()) {
    stream << ", camera_id="
           << (image.HasCameraId() ? std::to_string(image.CameraId())
                                   : "Invalid");
  } else {
    stream << ", camera=Camera(camera_id=" << std::to_string(image.CameraId())
           << ")";
  }
  stream << ", name=\"" << image.Name() << "\""
         << ", has_pose=" << image.HasPose()
         << ", triangulated=" << image.NumPoints3D() << "/"
         << image.NumPoints2D() << ")";
  return stream;
}

}  // namespace colmap
