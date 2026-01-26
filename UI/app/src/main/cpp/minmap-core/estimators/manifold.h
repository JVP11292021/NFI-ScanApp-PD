#pragma once

#include <ceres/ceres.h>
#include <ceres/rotation.h>

namespace colmap {

#include <cmath>

inline void SetQuaternionManifold(ceres::Problem* problem, double* quat_xyzw) {
#if CERES_VERSION_MAJOR >= 3 || \
    (CERES_VERSION_MAJOR == 2 && CERES_VERSION_MINOR >= 1)
  problem->SetManifold(quat_xyzw, new ceres::EigenQuaternionManifold);
#else
  problem->SetParameterization(quat_xyzw,
                               new ceres::EigenQuaternionParameterization);
#endif
}

inline void SetSubsetManifold(int size,
                              const std::vector<int>& constant_params,
                              ceres::Problem* problem,
                              double* params) {
#if CERES_VERSION_MAJOR >= 3 || \
    (CERES_VERSION_MAJOR == 2 && CERES_VERSION_MINOR >= 1)
  problem->SetManifold(params,
                       new ceres::SubsetManifold(size, constant_params));
#else
  problem->SetParameterization(
      params, new ceres::SubsetParameterization(size, constant_params));
#endif
}

template <int size>
inline void SetSphereManifold(ceres::Problem* problem, double* params) {
#if CERES_VERSION_MAJOR >= 3 || \
    (CERES_VERSION_MAJOR == 2 && CERES_VERSION_MINOR >= 1)
  problem->SetManifold(params, new ceres::SphereManifold<size>);
#else
  problem->SetParameterization(
      params, new ceres::HomogeneousVectorParameterization(size));
#endif
}

inline int ParameterBlockTangentSize(const ceres::Problem& problem,
                                     const double* param) {
#if CERES_VERSION_MAJOR >= 3 || \
    (CERES_VERSION_MAJOR == 2 && CERES_VERSION_MINOR >= 1)
  return problem.ParameterBlockTangentSize(param);
#else
  return problem.ParameterBlockLocalSize(param);
#endif
}

}  // namespace colmap
