#include "pose_prior.h"

namespace colmap {

std::ostream& operator<<(std::ostream& stream, const PosePrior& prior) {
  const static Eigen::IOFormat kVecFmt(
      Eigen::StreamPrecision, Eigen::DontAlignCols, ", ", ", ");
  stream << "PosePrior(position=[" << prior.position.format(kVecFmt)
         << "], position_covariance=["
         << prior.position_covariance.format(kVecFmt) << "], coordinate_system="
         << PosePrior::CoordinateSystemToString(prior.coordinate_system) << ")";
  return stream;
}

}  // namespace colmap
