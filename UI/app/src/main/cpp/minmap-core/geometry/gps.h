#pragma once

#include "../util/types.h"

#include <vector>

#include <Eigen/Core>

namespace colmap {

// Transform ellipsoidal GPS coordinates to Cartesian GPS coordinate
// representation and vice versa.
class GPSTransform {
public:
    enum class Ellipsoid {
        GRS80 = 0,
        WGS84 = 1,
    };

    constexpr std::string_view EllipsoidToString(Ellipsoid v) {
        switch (v) {
        case Ellipsoid::GRS80: return "GRS80";
        case Ellipsoid::WGS84: return "WGS84";
        default: return "UNKNOWN";
        }
    }

    inline Ellipsoid EllipsoidFromString(std::string_view s) {
        if (s == "GRS80") return Ellipsoid::GRS80;
        if (s == "WGS84") return Ellipsoid::WGS84;
        throw std::runtime_error("Invalid Ellipsoid: " + std::string(s));
    }

    explicit GPSTransform(Ellipsoid ellipsoid = Ellipsoid::GRS80);

    std::vector<Eigen::Vector3d> EllipsoidToECEF(
        const std::vector<Eigen::Vector3d>& lat_lon_alt) const;

    std::vector<Eigen::Vector3d> ECEFToEllipsoid(
        const std::vector<Eigen::Vector3d>& xyz_in_ecef) const;

    // Convert GPS (lat / lon / alt) to ENU coords. with ref_lat and ref_lon
    // defining the origin of the ENU frame
    std::vector<Eigen::Vector3d> EllipsoidToENU(
        const std::vector<Eigen::Vector3d>& lat_lon_alt,
        double ref_lat,
        double ref_lon) const;

    std::vector<Eigen::Vector3d> ECEFToENU(
        const std::vector<Eigen::Vector3d>& xyz_in_ecef,
        double ref_lat,
        double ref_lon) const;

    std::vector<Eigen::Vector3d> ENUToEllipsoid(
        const std::vector<Eigen::Vector3d>& xyz_in_enu,
        double ref_lat,
        double ref_lon,
        double ref_alt) const;

    std::vector<Eigen::Vector3d> ENUToECEF(
        const std::vector<Eigen::Vector3d>& xyz_in_enu,
        double ref_lat,
        double ref_lon,
        double ref_alt) const;

    // Converts GPS (lat / lon / alt) to UTM coordinates.
    // Returns a pair of the converted coordinates and the zone number.
    // If the points span multiple zones, the zone with the most points
    // is chosen as the reference frame.
    //
    // The conversion uses a 4th-order expansion formula. The easting offset is
    // 500 km, and the northing offset is 10,000 km for the Southern Hemisphere.
    std::pair<std::vector<Eigen::Vector3d>, int> EllipsoidToUTM(
        const std::vector<Eigen::Vector3d>& lat_lon_alt) const;

    // Converts UTM coords to GPS (lat / lon / alt).
    // Requires the zone number and hemisphere (true for north, false for south).
    std::vector<Eigen::Vector3d> UTMToEllipsoid(
        const std::vector<Eigen::Vector3d>& xyz_in_utm,
        int zone,
        bool is_north) const;

    private:
    // Semimajor axis.
    double a_;
    // Semiminor axis.
    double b_;
    // Flattening.
    double f_;
    // Numerical eccentricity.
    double e2_;
};

}  // namespace colmap
