#include "CvUtils.hpp"

#include <glm/ext/matrix_transform.hpp>
#include <filesystem>

SFM_NS_B

glm::mat4 cvToEngineRotation() {
    // Convert CV camera (Z up, X forward)
    // to engine camera (Y up, -Z forward)
    glm::mat4 R(1.0f);

    // Rotate +90° around X: Z-up → Y-up
    R = glm::rotate(R, glm::radians(90.0f), glm::vec3(1, 0, 0));

    return R;
}

glm::dmat3 getRotation(const float x_angle, const float y_angle, const float z_angle) {
    glm::dmat4 rotation(1.0f);
    rotation = glm::rotate(rotation, static_cast<double>(z_angle), glm::dvec3(0.0, 0.0, 1.0));
    rotation = glm::rotate(rotation, static_cast<double>(y_angle), glm::dvec3(0.0, 1.0, 0.0));
    rotation = glm::rotate(rotation, static_cast<double>(x_angle), glm::dvec3(1.0, 0.0, 0.0));
    return glm::dmat3(rotation);
}


glm::vec3 getGlmColorFromImage(const cv::Mat& img, const cv::KeyPoint& point, double image_scale) {
    // cv::Vec3b vec = img.at<cv::Vec3b>(point.pt * image_scale);
    // return glm::vec3(vec[2]/255.0, vec[1]/255.0, vec[0]/255.0);
    return getGlmColorFromImage(img, point.pt, image_scale);
}

glm::vec3 getGlmColorFromImage(const cv::Mat& img, const cv::Point2f& pt, double image_scale) {
    cv::Vec3b vec = img.at<cv::Vec3b>(pt * image_scale);
    return glm::vec3(vec[2] / 255.0, vec[1] / 255.0, vec[0] / 255.0);
}

cv::Mat loadImage(const ImageData& im_data, double scale_factor) {
    std::filesystem::path full_image_path = std::filesystem::path(im_data.image_dir)
        / std::filesystem::path(im_data.filename);
    cv::Mat img = cv::imread(full_image_path.string().c_str());
    if (scale_factor == 1.0) return img;
    cv::resize(img, img, cv::Size(), scale_factor, scale_factor);
    return img;
}
void imShow(const std::string& window_name, const cv::Mat& img, const double scale) {
    cv::Mat resized_img;
    cv::resize(img, resized_img, cv::Size(), scale, scale /*, cv::INTER_AREA*/);
    cv::imshow(window_name, resized_img);
}

void drawKeypointsWithResize(const cv::Mat& input_img,
    const std::vector<cv::KeyPoint>& kpoints,
    cv::Mat& out_img,
    const double scale) {
    std::vector<cv::KeyPoint> kpoints_scaled(kpoints);
    for (size_t k = 0; k < kpoints_scaled.size(); ++k) {
        kpoints_scaled[k].pt.x = static_cast<int>(kpoints_scaled[k].pt.x * scale);
        kpoints_scaled[k].pt.y = static_cast<int>(kpoints_scaled[k].pt.y * scale);
    }
    cv::Mat img_resized;
    using namespace std::chrono;
    auto t1_0 = high_resolution_clock::now();
    cv::resize(input_img, img_resized, cv::Size(), scale, scale, cv::INTER_AREA);
    auto t1_1 = high_resolution_clock::now();
    cv::drawKeypoints(input_img, kpoints_scaled, out_img);
    auto t1_2 = high_resolution_clock::now();
}

void drawMatchesWithResize(const cv::Mat& img1,
    const std::vector<cv::KeyPoint>& kpoints1,
    const cv::Mat& img2,
    const std::vector<cv::KeyPoint>& kpoints2,
    cv::Mat& img_matches,
    const double scale,
    const std::vector<cv::DMatch>& match) {
    std::vector<cv::KeyPoint> kpoints1_scaled(kpoints1);
    std::vector<cv::KeyPoint> kpoints2_scaled(kpoints2);
    for (size_t k = 0; k < kpoints1_scaled.size(); ++k) {
        kpoints1_scaled[k].pt.x = static_cast<int>(kpoints1_scaled[k].pt.x * scale);
        kpoints1_scaled[k].pt.y = static_cast<int>(kpoints1_scaled[k].pt.y * scale);
    }
    for (size_t k = 0; k < kpoints2_scaled.size(); ++k) {
        kpoints2_scaled[k].pt.x = static_cast<int>(kpoints2_scaled[k].pt.x * scale);
        kpoints2_scaled[k].pt.y = static_cast<int>(kpoints2_scaled[k].pt.y * scale);
    }
    cv::Mat img1_resized, img2_resized;
    if (match.size() == 0 && kpoints1.size() == kpoints2.size()) {
        std::vector<cv::DMatch> matches(kpoints1.size());
        for (size_t i = 0; i < kpoints1.size(); ++i) {
            matches[i].queryIdx = i;
            matches[i].trainIdx = i;
        }
        cv::drawMatches(img1, kpoints1_scaled, img2, kpoints2_scaled,
            matches, img_matches, cv::Scalar::all(-1),
            cv::Scalar::all(-1), std::vector<char>(),
            cv::DrawMatchesFlags::DEFAULT);
    }
    else {
        cv::drawMatches(img1, kpoints1_scaled, img2, kpoints2_scaled,
            match, img_matches, cv::Scalar::all(-1),
            cv::Scalar::all(-1), std::vector<char>(),
            cv::DrawMatchesFlags::DEFAULT);
    }

}

void imShowMatchesWithResize(const cv::Mat& img1,
    const std::vector<cv::KeyPoint>& kpoints1,
    const cv::Mat& img2,
    const std::vector<cv::KeyPoint>& kpoints2,
    const std::vector<cv::DMatch>& match,
    const double scale,
    const int win_x,
    const int win_y) {

    std::vector<cv::KeyPoint> kpoints1m, kpoints2m;
    if (!match.empty()) {
        for (size_t i = 0; i < match.size(); ++i) {
            kpoints1m.push_back(kpoints1[match[i].queryIdx]);
            kpoints2m.push_back(kpoints2[match[i].trainIdx]);
        }
    }
    else {
        kpoints1m = kpoints1;
        kpoints2m = kpoints2;
    }

    cv::Mat img1_points, img2_points, img_matches;
    drawKeypointsWithResize(img1, kpoints1m, img1_points, scale);
    drawKeypointsWithResize(img2, kpoints2m, img2_points, scale);
    imShow("img2", img2_points);
    cv::moveWindow("img2", win_x, win_y);
    imShow("img1", img1_points);
    cv::moveWindow("img1", win_x + img2_points.size().width, win_y);

    if (!match.empty()) {

        std::vector<cv::DMatch> new_match;
        for (size_t i = 0; i < match.size(); ++i) {
            new_match.push_back(cv::DMatch(i, i, match[i].distance));
        }
        drawMatchesWithResize(img1, kpoints1m,
            img2, kpoints2m,
            img_matches,
            scale, new_match);
        imShow("img_matches", img_matches);
        cv::moveWindow("img_matches", win_x, win_y + img1_points.size().height + 12);
    }

}


bool isPairInOrder(const ImageData& im_data1, const ImageData& im_data2) {
    // TODO: Optimize it by comparing elements rather than combined strings
    auto im1_stem = std::filesystem::path(im_data1.filename).stem();
    auto im2_stem = std::filesystem::path(im_data2.filename).stem();
    std::string cache1_name = im_data1.record + "_"
        + std::to_string(im_data1.camera_num) + "_"
        + im1_stem.string();
    std::string cache2_name = im_data2.record + "_"
        + std::to_string(im_data2.camera_num) + "_"
        + im2_stem.string();
    return cache1_name < cache2_name;
}

void keyPointToPointVec(const std::vector<cv::KeyPoint>& kpoints, std::vector<cv::Point2f>& points) {
    for (size_t i = 0; i < kpoints.size(); ++i) {
        points.push_back(kpoints[i].pt);
    }
}

void keyPointsToPointVec(const std::vector<cv::KeyPoint>& kpoints1,
    const std::vector<cv::KeyPoint>& kpoints2,
    const std::vector<cv::DMatch>& match,
    std::vector<cv::Point2f>& points1,
    std::vector<cv::Point2f>& points2) {
    for (size_t i = 0; i < match.size(); ++i) {
        points1.push_back(kpoints1[match[i].queryIdx].pt);
        points2.push_back(kpoints2[match[i].trainIdx].pt);
    }
}


SFM_NS_E