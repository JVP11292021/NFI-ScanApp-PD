#ifndef SFM_CV_UTILS_H
#define SFM_CV_UTILS_H

#include "ns.hpp"
#include <glm/glm.hpp>
#include <opencv2/opencv.hpp>
#include <corecrt_math_defines.h>

SFM_NS_B

struct ImageData {
	std::string record;
	std::string image_dir;
	int camera_num;
	std::string filename;
	std::vector<double> coords;
};

enum class ThreadStatus {
	PROCESSING = 0,
	FINISH
};


inline double deg2rad(const double d) {
	return d / 180.0 * M_PI;
}

glm::mat4 cvToEngineRotation();

glm::dmat3 getRotation(const float x_angle, const float y_angle, const float z_angle);
glm::vec3 getGlmColorFromImage(const cv::Mat& img, const cv::KeyPoint& point, double image_scale);
glm::vec3 getGlmColorFromImage(const cv::Mat& img, const cv::Point2f& pt, double image_scale = 1.0);

cv::Mat loadImage(const ImageData& im_data, double scale_factor = 1.0);
bool isPairInOrder(const ImageData& im_data1, const ImageData& im_data2);
void imShow(const std::string& window_name, const cv::Mat& img, const double scale = 1.0);
void drawKeypointsWithResize(const cv::Mat& input_img, const std::vector<cv::KeyPoint>& kpoints, cv::Mat& out_img, const double scale = 1.0);
void drawMatchesWithResize(const cv::Mat& img1,
    const std::vector<cv::KeyPoint>& kpoints1,
    const cv::Mat& img2,
    const std::vector<cv::KeyPoint>& kpoints2,
    cv::Mat& img_matches,
    const double scale = 1.0,
    const std::vector<cv::DMatch>& match = std::vector<cv::DMatch>());
void imShowMatchesWithResize(const cv::Mat& img1,
    const std::vector<cv::KeyPoint>& kpoints1,
    const cv::Mat& img2,
    const std::vector<cv::KeyPoint>& kpoints2,
    const std::vector<cv::DMatch>& match,
    const double scale = 1.0,
    const int win_x = 0,
    const int win_y = 10);

void keyPointToPointVec(const std::vector<cv::KeyPoint>& kpoints, std::vector<cv::Point2f>& points);
void keyPointsToPointVec(const std::vector<cv::KeyPoint>& kpoints1,
	const std::vector<cv::KeyPoint>& kpoints2,
	const std::vector<cv::DMatch>& match,
	std::vector<cv::Point2f>& points1,
	std::vector<cv::Point2f>& points2);

SFM_NS_E

#endif // SFM_CV_UTILS_H

