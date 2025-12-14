#include <opencv2/opencv.hpp>

#include <SfM.hpp>

#include <filesystem>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

namespace fs = std::filesystem;

/* =========================
   Utility: Load images
   ========================= */
std::vector<cv::Mat> loadImagesFromDirectory(
    const std::string& directory,
    std::vector<std::string>& imageNames)
{
    std::vector<cv::Mat> images;
    imageNames.clear();

    if (!fs::exists(directory)) {
        throw std::runtime_error("Image directory does not exist");
    }

    std::vector<fs::path> files;
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (!entry.is_regular_file()) continue;

        auto ext = entry.path().extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        if (ext == ".jpg" || ext == ".png" || ext == ".jpeg") {
            files.push_back(entry.path());
        }
    }

    std::sort(files.begin(), files.end());

    for (const auto& path : files) {
        cv::Mat img = cv::imread(path.string(), cv::IMREAD_GRAYSCALE);
        if (img.empty()) {
            std::cerr << "Failed to load: " << path << std::endl;
            continue;
        }
        images.push_back(img);
        imageNames.push_back(path.filename().string());
    }

    std::cout << "Loaded " << images.size() << " images\n";
    return images;
}

/* =========================
   Minimal SfM test
   ========================= */
int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cerr << "Usage: ./sfm_test <image_directory>\n";
        return EXIT_FAILURE;
    }

    std::string imageDir = argv[1];

    std::vector<std::string> imageNames;
    std::vector<cv::Mat> images;

    try {
        images = loadImagesFromDirectory(imageDir, imageNames);
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    if (images.size() < 2) {
        std::cerr << "Need at least two images for SfM\n";
        return EXIT_FAILURE;
    }

    /* =========================
       Camera intrinsics (example)
       ========================= */
    double fx = 800;
    double fy = 800;
    double cx = images[0].cols * 0.5;
    double cy = images[0].rows * 0.5;

    cv::Mat K = (cv::Mat_<double>(3, 3) <<
        fx, 0, cx,
        0, fy, cy,
        0, 0, 1
        );

    std::cout << "Camera K:\n" << K << "\n";

    /* =========================
       Feature extraction
       ========================= */
    cv::Ptr<cv::SIFT> sift = cv::SIFT::create(4000);

    std::vector<std::vector<cv::KeyPoint>> keypoints(images.size());
    std::vector<cv::Mat> descriptors(images.size());

    for (size_t i = 0; i < images.size(); ++i) {
        sift->detectAndCompute(images[i], cv::noArray(),
            keypoints[i], descriptors[i]);
        std::cout << "Image " << i << ": "
            << keypoints[i].size() << " keypoints\n";
    }

    /* =========================
       Match first two images
       ========================= */
    cv::BFMatcher matcher(cv::NORM_L2);
    std::vector<std::vector<cv::DMatch>> knnMatches;
    matcher.knnMatch(descriptors[0], descriptors[1], knnMatches, 2);

    std::vector<cv::DMatch> goodMatches;
    for (auto& m : knnMatches) {
        if (m.size() >= 2 && m[0].distance < 0.75f * m[1].distance) {
            goodMatches.push_back(m[0]);
        }
    }

    std::cout << "Good matches: " << goodMatches.size() << "\n";

    if (goodMatches.size() < 50) {
        std::cerr << "Not enough matches for pose estimation\n";
        return EXIT_FAILURE;
    }

    /* =========================
       Extract matched points
       ========================= */
    std::vector<cv::Point2f> pts1, pts2;
    for (auto& m : goodMatches) {
        pts1.push_back(keypoints[0][m.queryIdx].pt);
        pts2.push_back(keypoints[1][m.trainIdx].pt);
    }

    /* =========================
       Essential matrix + pose
       ========================= */
    cv::Mat mask;
    cv::Mat E = cv::findEssentialMat(
        pts1, pts2, K, cv::RANSAC, 0.999, 1.0, mask);

    if (E.empty()) {
        std::cerr << "Failed to compute essential matrix\n";
        return EXIT_FAILURE;
    }

    cv::Mat R, t;
    int inliers = cv::recoverPose(E, pts1, pts2, K, R, t, mask);

    std::cout << "Recovered pose with " << inliers << " inliers\n";
    std::cout << "R:\n" << R << "\n";
    std::cout << "t:\n" << t.t() << "\n";

    /* =========================
       Triangulation
       ========================= */
    cv::Mat P1 = K * cv::Mat::eye(3, 4, CV_64F);
    cv::Mat P2(3, 4, CV_64F);
    R.copyTo(P2(cv::Rect(0, 0, 3, 3)));
    t.copyTo(P2.col(3));
    P2 = K * P2;

    cv::Mat points4D;
    cv::triangulatePoints(P1, P2, pts1, pts2, points4D);

    std::vector<cv::Point3f> points3D;
    for (int i = 0; i < points4D.cols; ++i) {
        cv::Mat x = points4D.col(i);
        x /= x.at<float>(3);
        points3D.emplace_back(
            x.at<float>(0),
            x.at<float>(1),
            x.at<float>(2)
        );
    }

    std::cout << "Triangulated " << points3D.size() << " points\n";

    /* =========================
       Done
       ========================= */
    std::cout << "SfM bootstrap successful\n";
    return EXIT_SUCCESS;
}
