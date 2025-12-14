#ifndef SFM3D_H
#define SFM3D_H

#include "ns.hpp"
#include "DSU.hpp"
#include "CameraSystem.hpp"

#include <opencv2/core.hpp>

#include <unordered_set>
#include <vector>
#include <map>

SFM_NS_B

// *******************************************************************
// STRUCT TYPE DEFINITIONS
// *******************************************************************

struct Features {
    std::vector<cv::KeyPoint> keypoints;
    cv::Mat descriptors;
};

struct CameraPose {
    cv::Mat R;   // 3x3
    cv::Mat t;   // 3x1
    bool registered = false;
};

struct Camera {
    cv::Mat K;   // intrinsics
    CameraPose pose;
};

struct WorldPoint3D {
    cv::Point3d xyz;
    int track_id;
};

struct Matches {
    int img1, img2;
    std::vector<cv::DMatch> matches;
};

// *******************************************************************
// TYPES
// *******************************************************************

using Obs = std::pair<int, int>; // (image_id, keypoint_id)

// *******************************************************************
// SfM Impl - SfM3D.cpp
// *******************************************************************

class SfM3D {
public:
    SfM3D(const std::vector<cv::Mat>& intrinsics);

public:
    void addImages(const std::vector<cv::Mat>& images);
    void extractFeatures();
    void matchFeatures();

    void reconstruct();   // FULL SfM pipeline

    std::vector<int> getCommonTracks(int i, int j);
    inline const std::vector<WorldPoint3D>& points() const { return map_; }
    inline const std::vector<Camera>& cameras() const { return cameras_; }
    inline const std::vector<Features> features() const { return features_; }
    inline const std::vector<Matches> matches() const { return matches_; }

private:
    // SfM steps
    void bootstrap();
    bool estimateInitialPose(
        int img1, int img2,
        cv::Mat& R, cv::Mat& t);
    int selectNextView();
    void registerNextView(int img);
    void triangulateTracks(int img1, int img2);
    void buildTracks();


    // Data
    std::vector<cv::Mat> images_;
    std::vector<Features> features_;
    std::vector<Matches> matches_;
    std::vector<Camera> cameras_;
    std::vector<WorldPoint3D> map_;
    DSU<Obs> tracks_;

    std::unordered_set<int> registered_views_;
};

SFM_NS_E

//namespace std {    
//template<>
//struct hash<sfm::Obs> {
//    size_t operator()(const sfm::Obs& o) const {
//        return std::hash<int>()(o.first) ^ (std::hash<int>()(o.second) << 1);
//    }
//};
//}

#endif // SFM3D_H