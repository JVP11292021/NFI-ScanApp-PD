#include "two_view_geometry.h"

namespace colmap {

void TwoViewGeometry::Invert() {
  F.transposeInPlace();
  E.transposeInPlace();
  H = H.inverse().eval();
  cam2_from_cam1 = Inverse(cam2_from_cam1);
  for (auto& match : inlier_matches) {
    std::swap(match.point2D_idx1, match.point2D_idx2);
  }
}

}  // namespace colmap
