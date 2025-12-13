#ifndef SFM3D_H
#define SFM3D_H

#include "ns.hpp"
#include "CvUtils.hpp"
#include "DSU.hpp"
#include "CameraSystem.hpp"

#include <opencv2/core.hpp>

#include <vector>
#include <map>
#include <unordered_set>
#include <mutex>
#include <condition_variable>

SFM_NS_B

// *******************************************************************
// STRUCT TYPE DEFINITIONS
// *******************************************************************

struct Features {
	std::vector<cv::KeyPoint> keypoints;
	cv::Mat descriptors;
};

struct ImagePair {
	std::int32_t first;
	std::int32_t second;
};

struct Matches {
	ImagePair image_index;
	std::vector<cv::DMatch> match;
};

struct CameraInfo {
	CameraSystemIntrinsics intr;
	glm::dvec3 translation;
	glm::dvec3 rotation_angles;
};

struct WorldPoint3D {
	cv::Point3d pt;
	std::map<std::int32_t, std::int32_t> views;
	std::int32_t component_id;
};

struct Point3DColor {
	Point3DColor() :
		pt{ 0.0 }, color{ 0.0 }, color_br{ 0.0 }, color_bl{ 0.0 }, color_tr{ 0.0 },
		color_tl{ 0.0 }, err{ 0.0 } {
	}
	glm::vec3 pt;
	glm::vec3 color;
	glm::vec3 color_br;
	glm::vec3 color_bl;
	glm::vec3 color_tr;
	glm::vec3 color_tl;
	double err;
};


// *******************************************************************
// TYPEDEFS
// *******************************************************************

typedef std::vector<WorldPoint3D> Map3D;
typedef std::map<std::int32_t, std::vector<std::pair<std::int32_t, Point3DColor> > > MapCameras;

// *******************************************************************
// FUNCTIONS - SfMFunctions.cpp
// *******************************************************************

double getCamerasDistance(const CameraInfo& camera_info1,
    const CameraInfo& camera_info2);

void calcFundamental(const CameraInfo& camera_info1, const CameraInfo& camera_info2, cv::Mat& fund);
cv::Mat calcFundamental(const CameraSystemIntrinsics& intr1, const ImageData& img_data1, const CameraSystemIntrinsics& intr2, const ImageData& img_data2);

void extractFeatures(cv::Mat img, Features& features);
void extractFeaturesAndCameraInfo(const std::string& image_path,
    const ImageData& im_data,
    const CameraSystemIntrinsics& intr,
    cv::Mat& img,
    Features& features,
    CameraInfo& camera_info);
void computeLineKeyPointsMatch(const Features& features1,
    const CameraInfo camera_info1,
    const Features& features2,
    const CameraInfo& camera_info2,
    Matches& matches);
void filterMatchByLineDistance(const Features& features1,
    const CameraInfo camera_info1,
    const Features& features2,
    const CameraInfo& camera_info2,
    Matches& matches,
    const double line_dist);

void getLineImagePoints(const cv::Mat& line, std::vector<cv::Point2f>& line_pts,
    const double image_width, const double image_height);

void getLineMatchedSURFKeypoints(const cv::Mat img1, std::vector<cv::KeyPoint>& keypoints1,
    const cv::Mat img2, std::vector<cv::KeyPoint>& keypoints2, const cv::Mat fund);

//void getMatchedSURFKeypoints(const cv::Mat img1, std::vector<cv::KeyPoint>& keypoints1,
//    const cv::Mat img2, std::vector<cv::KeyPoint>& keypoints2, const cv::Mat fund = cv::Mat());

cv::Mat getProjMatrix(const CameraInfo& camera_info);
cv::Mat getRotationTranslationTransform(const CameraInfo& camera_info);

void triangulatePoints(const CameraSystemIntrinsics& intr1, const ImageData& img_data1, const std::vector<cv::Point2f>& points1,
    const CameraSystemIntrinsics& intr2, const ImageData& img_data2, const std::vector<cv::Point2f>& points2, cv::Mat& points3d);

void triangulatePoints(const CameraInfo& camera_info1, const std::vector<cv::Point2f>& points1,
    const CameraInfo& camera_info2, const std::vector<cv::Point2f>& points2,
    cv::Mat& points3d);

std::vector<double> getReprojectionErrors(
    const std::vector<cv::Point2f>& points,
    const cv::Mat& proj,
    const cv::Mat& points3d);
double getReprojectionError(
    const Map3D& map,
    const std::vector<CameraInfo>& cameras,
    const std::vector<Features>& features);
std::vector<double> getReprojectionErrors(
    const Map3D& map,
    const std::vector<CameraInfo>& cameras,
    const std::vector<Features>& features);
double getReprojectionError(const WorldPoint3D& point3d,
    const std::vector<CameraInfo>& cameras,
    const std::vector<Features>& features);
std::vector<double> getZDistanceFromCamera(const CameraInfo& camera_info,
    const cv::Mat& points3d);
void removeOutliersByError(Map3D& map,
    const std::vector<CameraInfo>& cameras,
    const std::vector<Features>& features,
    const float percentile);

Map3D reduceMapByError(const Map3D& map,
    const std::vector<CameraInfo>& cameras,
    const std::vector<Features>& features,
    const float ratio);

std::int32_t getNextBestView(const Map3D& map,
    const std::unordered_set<std::int32_t>& views,
    DSU<std::pair<std::int32_t, std::int32_t> >& ccomp,
    const std::vector<Matches>& image_matches,
    const std::map<std::pair<std::int32_t, std::int32_t>, std::int32_t>& matches_index);

std::int32_t getNextBestViewByViews(const Map3D& map,
    const std::unordered_set<std::int32_t>& todo_views,
    const std::unordered_set<std::int32_t>& used_views,
    const std::vector<Matches>& image_matches,
    const std::map<std::pair<std::int32_t, std::int32_t>, std::int32_t>& matches_index);


// TODO: Make DSU const
void mergeToTheMap(Map3D& map,
    const Map3D& local_map,
    DSU<std::pair<std::int32_t, std::int32_t> >& ccomp);

void mergeToTheMapImproved(Map3D& map,
    const Map3D& local_map,
    DSU<std::pair<std::int32_t, std::int32_t> >& ccomp);

void combineMapComponents(Map3D& map, const double max_keep_dist);
void mergeAndCombinePoints(Map3D& map,
    const Map3D& local_map,
    const double max_keep_dist);


void getKeyPointColors(const cv::Mat& img,
    const cv::KeyPoint& point,
    Point3DColor& p3d,
    const bool add_colors = false,
    double dangle = 0.0,
    double image_scale = 1.0);

// *******************************************************************
// OPERATORS
// *******************************************************************
inline bool operator==(const ImagePair& ip1, const ImagePair& ip2) {
	return (ip1.first == ip2.first && ip1.second == ip2.second);
}

inline bool operator<(const ImagePair& ip1, const ImagePair& ip2) {
	return (ip1.first != ip2.first ? ip1.first < ip2.first : ip1.second < ip2.second);
}

std::ostream& operator<<(std::ostream& os, const ImagePair& ip);
std::ostream& operator<<(std::ostream& os, const WorldPoint3D& wp);

// *******************************************************************
// SfM Impl - SfM3D.cpp
// *******************************************************************

class SfM3D {
public:
    typedef std::pair<std::int32_t, std::int32_t> IntPair;


    enum SfMStatus {
        RECONSTRUCTION,
        FINISH
    };

    SfM3D() : SfM3D{ std::vector<CameraSystemIntrinsics>() } {}
    explicit SfM3D(std::vector<CameraSystemIntrinsics> camera_intrs)
        : intrinsics_(camera_intrs),
        proc_status_(RECONSTRUCTION),
        vis_version_(0),
        repr_error_thresh(1.0),
        max_merge_dist(1.0
    ) {}

public:
    void addImages(const std::vector<ImageData>& camera1_images,
        const std::vector<ImageData>& camera2_images,
        const bool make_pairs = true, const std::int32_t look_back = 5);
    void extractFeatures();
    void matchImageFeatures(const std::int32_t skip_thresh = 10,
        const double max_line_dist = 10.0,
        const bool use_cache = true);

    void initReconstruction();
    void reconstructAll();

    bool getMapPointsVec(std::vector<Point3DColor>& glm_points);
    bool getMapCamerasWithPointsVec(MapCameras& map_cameras);
    void getMapPointsAndCameras(std::vector<Point3DColor>& glm_points, MapCameras& map_cameras, int& last_version);

    cv::Mat getImage(std::int32_t cam_id, bool full_size = false) const;
    CameraInfo getCameraInfo(std::int32_t cam_id) const;
    cv::KeyPoint getKeypoint(std::int32_t cam_id, std::int32_t point_id) const;

    std::int32_t imageCount() const;
    std::int32_t mapSize() const;

    void restoreImages();
    void clearImages();

    void setProcStatus(SfMStatus proc_status);
    bool isFinished();
    void emitMapUpdate();


    // Debug methods
    void print(std::ostream& os = std::cout) const;
    void printFinalStats();
    void showFeatures(std::int32_t img_id);
    void showMatches(const Matches& matches);
    void showMatchesLineConstraints(const Matches& matches, const double line_dist);

public:
    double repr_error_thresh;
    double max_merge_dist;
    double resize_scale = 0.08;

private:
    void generateAllPairs();

    void triangulatePointsFromViews(const std::int32_t first_id, const std::int32_t second_id, Map3D& map);
    void optimizeCurrentMap() { this->optimizeMap(map_); }
    void optimizeMap(Map3D& map);


    void reconstructNextView(const std::int32_t next_img_id);
    void reconstructNextViewPair(const std::int32_t first_id, const std::int32_t second_id);

    int findMaxSizeMatch(const bool within_todo_views = false) const;

    bool isPairInOrder(const std::int32_t p1, const std::int32_t p2);


private:
    // Data Initial
    std::vector<CameraSystemIntrinsics> intrinsics_;
    std::vector<ImageData> image_data_;
    std::vector<CameraInfo> cameras_;
    std::vector<cv::Mat> images_;


    std::vector<cv::Mat> images_resized_;

    // Pre-processing & Feature Extraction
    std::vector<Features> image_features_;
    std::vector<ImagePair> image_pairs_;

    // Matching
    DSU<IntPair> ccomp_;
    std::vector<Matches> image_matches_;
    std::map<IntPair, int> matches_index_;


    // Reconstruction
    std::unordered_set<int> used_views_;
    std::unordered_set<int> todo_views_;
    Map3D map_;

    std::mutex map_mutex;
    std::condition_variable map_update_;
    std::atomic<long> vis_version_;

    std::atomic<SfMStatus> proc_status_;
};


std::ostream& operator<<(std::ostream& os, const SfM3D& sfm);

struct SuperCostFunctor {
    template <typename T> bool operator()(const T* const x, T* residual) const {
        residual[0] = 10.0 - x[0];
        return true;
    }
};

SFM_NS_E


#endif // SFM3D_H