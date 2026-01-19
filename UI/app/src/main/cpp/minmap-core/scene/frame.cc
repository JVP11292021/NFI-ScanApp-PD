#include "frame.h"

namespace colmap {

void Frame::SetCamFromWorld(camera_t camera_id, const Rigid3d& cam_from_world) {
  THROW_CHECK_NOTNULL(rig_ptr_);
  const sensor_t sensor_id(SensorType::CAMERA, camera_id);
  if (rig_ptr_->IsRefSensor(sensor_id)) {
    SetRigFromWorld(cam_from_world);
  } else {
    const Rigid3d& cam_from_rig = rig_ptr_->SensorFromRig(sensor_id);
    SetRigFromWorld(Inverse(cam_from_rig) * cam_from_world);
  }
}

std::ostream& operator<<(std::ostream& stream, const Frame& frame) {
  stream << "Frame(frame_id=" << frame.FrameId() << ", rig_id=";
  if (frame.HasRigId()) {
    if (frame.RigId() == kInvalidRigId) {
      stream << "Invalid";
    } else {
      stream << frame.RigId();
    }
  } else {
    stream << "Unknown";
  }
  stream << ", has_pose=" << frame.HasPose() << ", data_ids=[";
  for (auto it = frame.DataIds().begin(); it != frame.DataIds().end();) {
    stream << "(" << it->sensor_id.type << ", " << it->sensor_id.id << ", "
           << it->id << ")";
    if (++it != frame.DataIds().end()) {
      stream << ", ";
    }
  }
  stream << "])";
  return stream;
}

}  // namespace colmap
