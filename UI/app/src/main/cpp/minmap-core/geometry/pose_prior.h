#pragma once

#include "../util/types.h"

#include <ostream>

#include <PoseLib/alignment.h>
#include <Eigen/Core>

namespace colmap {

struct PosePrior {
 public:
     enum class CoordinateSystem {
         UNDEFINED = -1,
         WGS84 = 0,
         CARTESIAN = 1,
     };

     static constexpr std::string_view CoordinateSystemToString(CoordinateSystem v) {
         switch (v) {
         case CoordinateSystem::UNDEFINED: return "UNDEFINED";
         case CoordinateSystem::WGS84:     return "WGS84";
         case CoordinateSystem::CARTESIAN: return "CARTESIAN";
         default: return "UNKNOWN";
         }
     }

     inline CoordinateSystem CoordinateSystemFromString(std::string_view s) {
         if (s == "UNDEFINED") return CoordinateSystem::UNDEFINED;
         if (s == "WGS84")     return CoordinateSystem::WGS84;
         if (s == "CARTESIAN") return CoordinateSystem::CARTESIAN;
         throw std::runtime_error("Invalid CoordinateSystem: " + std::string(s));
     }

    Eigen::Vector3d position =
        Eigen::Vector3d::Constant(std::numeric_limits<double>::quiet_NaN());
    Eigen::Matrix3d position_covariance =
        Eigen::Matrix3d::Constant(std::numeric_limits<double>::quiet_NaN());
    CoordinateSystem coordinate_system = CoordinateSystem::UNDEFINED;

    PosePrior() = default;
    explicit PosePrior(const Eigen::Vector3d& position) : position(position) {}
    PosePrior(const Eigen::Vector3d& position, const CoordinateSystem system)
        : position(position), coordinate_system(system) {}
    PosePrior(const Eigen::Vector3d& position, const Eigen::Matrix3d& covariance)
        : position(position), position_covariance(covariance) {}
    PosePrior(const Eigen::Vector3d& position,
            const Eigen::Matrix3d& covariance,
            const CoordinateSystem system)
        : position(position),
        position_covariance(covariance),
        coordinate_system(system) {}

    inline bool IsValid() const { return position.allFinite(); }
    inline bool IsCovarianceValid() const {
    return position_covariance.allFinite();
    }

    inline bool operator==(const PosePrior& other) const;
    inline bool operator!=(const PosePrior& other) const;
};

std::ostream& operator<<(std::ostream& stream, const PosePrior& prior);

bool PosePrior::operator==(const PosePrior& other) const {
  return coordinate_system == other.coordinate_system &&
         position == other.position &&
         position_covariance == other.position_covariance;
}

bool PosePrior::operator!=(const PosePrior& other) const {
  return !(*this == other);
}

}  // namespace colmap
