#include "SfM.hpp"

#include <algorithm>
#include <chrono>
#include <filesystem>

#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

SFM_NS_B

double getCamerasDistance(const CameraInfo& camera_info1,
    const CameraInfo& camera_info2) {
    return glm::distance(camera_info1.translation, camera_info2.translation);
}

void calcFundamental(const CameraInfo& camera_info1, const CameraInfo& camera_info2, cv::Mat& fund) {
    glm::dmat3 r1 = getRotation(camera_info1.rotation_angles[0], camera_info1.rotation_angles[1], camera_info1.rotation_angles[2]);

    glm::dmat3 r2 = getRotation(camera_info2.rotation_angles[0], camera_info2.rotation_angles[1], camera_info2.rotation_angles[2]);

    glm::dmat3 k1 = camera_info1.intr.getCameraMatrix();
    glm::dmat3 k2 = camera_info2.intr.getCameraMatrix();

    glm::dvec3 t1 = camera_info1.translation;
    glm::dvec3 t2 = camera_info2.translation;
    glm::dvec3 b = t2 - t1;
    glm::dmat3x3 sb;
    sb[0] = glm::dvec3(0.0, b[2], -b[1]);
    sb[1] = glm::dvec3(-b[2], 0.0, b[0]);
    sb[2] = glm::dvec3(b[1], -b[0], 0.0);

    glm::dmat3x3 f = glm::transpose(glm::inverse(k1)) * glm::transpose(r1) * sb * r2 * glm::inverse(k2);

    fund.create(3, 3, CV_64F);
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            fund.at<double>(row, col) = f[col][row];
        }
    }


}

cv::Mat calcFundamental(const CameraSystemIntrinsics& intr1, const ImageData& img_data1, const CameraSystemIntrinsics& intr2, const ImageData& img_data2) {

    CameraInfo camera_info1, camera_info2;

    camera_info1.intr = intr1;
    camera_info1.rotation_angles[0] = img_data1.coords[0];
    camera_info1.rotation_angles[1] = img_data1.coords[1];
    camera_info1.rotation_angles[2] = img_data1.coords[2];
    camera_info1.translation[0] = img_data1.coords[3];
    camera_info1.translation[1] = img_data1.coords[4];
    camera_info1.translation[2] = img_data1.coords[5];

    camera_info2.intr = intr2;
    camera_info2.rotation_angles[0] = img_data2.coords[0];
    camera_info2.rotation_angles[1] = img_data2.coords[1];
    camera_info2.rotation_angles[2] = img_data2.coords[2];
    camera_info2.translation[0] = img_data2.coords[3];
    camera_info2.translation[1] = img_data2.coords[4];
    camera_info2.translation[2] = img_data2.coords[5];

    cv::Mat fund;
    calcFundamental(camera_info1, camera_info2, fund);

    return fund;

}

void getFeatureExtractionRegion(const cv::Mat& img, cv::Mat& mask) {
    mask = cv::Mat::zeros(img.size(), CV_8UC1);
    //mask.zeros
    cv::Point mask_points[1][4];
    int xmin = (105.0 / 2452.0) * img.size().width;
    int ymin = (90.0 / 2056.0) * img.size().height;
    int xmax = (2356.0 / 2452.0) * img.size().width;
    int ymax = (1956.0 / 2056.0) * img.size().height;
    mask_points[0][0] = cv::Point(xmin, ymin);
    mask_points[0][1] = cv::Point(xmax, ymin);
    mask_points[0][2] = cv::Point(xmax, ymax);
    mask_points[0][3] = cv::Point(xmin, ymax);

    const cv::Point* mpt[1] = { mask_points[0] };
    int npt[] = { 4 };
    cv::fillPoly(mask, mpt, npt, 1, cv::Scalar(255, 0, 0), cv::LINE_8);
}

void getLineImagePoints(const cv::Mat& line, std::vector<cv::Point2f>& line_pts, const double image_width, const double image_height) {
    if (abs(line.at<double>(1)) > 10e-9) {
        cv::Point2f p1(0.0, -line.at<double>(2) / line.at<double>(1));
        if (p1.y >= 0.0 && p1.y <= image_height && line_pts.size() < 2) {
            line_pts.push_back(p1);
        }
        cv::Point2f p2(image_width, -(line.at<double>(2) + line.at<double>(0) * image_width) / line.at<double>(1));
        if (p2.y >= 0.0 && p2.y <= image_height && line_pts.size() < 2) {
            line_pts.push_back(p2);
        }
    }
    if (abs(line.at<double>(0)) > 10e-9 && line_pts.size() < 2) {
        cv::Point2f p3(-line.at<double>(2) / line.at<double>(0), 0.0);
        if (p3.x >= 0.0 && p3.x <= image_width && line_pts.size() < 2) {
            line_pts.push_back(p3);
        }
        cv::Point2f p4(-(line.at<double>(2) + line.at<double>(1) * image_height) / line.at<double>(0), image_height);
        if (p4.x >= 0.0 && p4.x <= image_width && line_pts.size() < 2) {
            line_pts.push_back(p4);
        }
    }
}

void extractFeatures(cv::Mat img, Features& features) {
    cv::Mat feature_mask;
    features.keypoints.clear();
    features.descriptors = cv::Mat();
    getFeatureExtractionRegion(img, feature_mask);
    cv::Ptr<cv::AKAZE> detector = cv::AKAZE::create();
    detector->detectAndCompute(img, feature_mask, features.keypoints, features.descriptors);
    std::cout << "features.keypoints: " << features.keypoints.size() << std::endl;
    std::cout << "features.descriptors: " << features.descriptors.size() << std::endl;
}

void extractFeaturesAndCameraInfo(const std::string& image_path,
    const ImageData& im_data,
    const CameraSystemIntrinsics& intr,
    cv::Mat& img,
    Features& features,
    CameraInfo& camera_info) {
    std::filesystem::path full_image_path = image_path / std::filesystem::path(im_data.filename);
    img = cv::imread(full_image_path.string().c_str());
    std::cout << "full_image_path = " << full_image_path.string() << std::endl;
    extractFeatures(img, features);
    camera_info.intr = intr;
    camera_info.rotation_angles[0] = im_data.coords[0];
    camera_info.rotation_angles[1] = im_data.coords[1];
    camera_info.rotation_angles[2] = im_data.coords[2];
    camera_info.translation[0] = im_data.coords[3];
    camera_info.translation[1] = im_data.coords[4];
    camera_info.translation[2] = im_data.coords[5];
}

void computeLineKeyPointsMatch(const Features& features1,
    const CameraInfo camera_info1,
    const Features& features2,
    const CameraInfo& camera_info2,
    Matches& matches
) {

    matches.match.clear();

    const cv::Mat& descriptors1 = features1.descriptors;
    const cv::Mat& descriptors2 = features2.descriptors;

    cv::Ptr<cv::DescriptorMatcher> matcher =
        cv::DescriptorMatcher::create(cv::DescriptorMatcher::BRUTEFORCE_HAMMING);

    std::vector<std::vector<cv::DMatch> > knnMatches;

    matcher->knnMatch(descriptors1, descriptors2, knnMatches, 2 /*, mask_int*/);

    const float ratio_thresh = 0.5f;
    for (int m = 0; m < knnMatches.size(); ++m) {
        if (knnMatches[m].size() < 2) continue; // no match for the points
        if (knnMatches[m][0].distance < ratio_thresh * knnMatches[m][1].distance) {
            cv::DMatch match = knnMatches[m][0];
            matches.match.push_back(match);
        }
    }
}

void filterMatchByLineDistance(const Features& features1,
    const CameraInfo camera_info1,
    const Features& features2,
    const CameraInfo& camera_info2,
    Matches& matches,
    const double line_dist
) {

    cv::Mat fund;
    calcFundamental(camera_info1, camera_info2, fund);

    auto match = matches.match.begin();
    while (match != matches.match.end()) {

        cv::Mat points2(3, 1, CV_64F);
        points2.at<double>(0, 0) = features2.keypoints[match->trainIdx].pt.x;
        points2.at<double>(1, 0) = features2.keypoints[match->trainIdx].pt.y;
        points2.at<double>(2, 0) = 1.0;

        cv::Mat points1(1, 3, CV_64F);
        points1.at<double>(0, 0) = features1.keypoints[match->queryIdx].pt.x;
        points1.at<double>(0, 1) = features1.keypoints[match->queryIdx].pt.y;
        points1.at<double>(0, 2) = 1.0;

        cv::Mat kp_l2 = fund * points2;
        double a = kp_l2.at<double>(0);
        double b = kp_l2.at<double>(1);
        double d = sqrt(a * a + b * b);

        cv::Mat dd_mat = points1 * kp_l2 / d;
        double dd = abs(dd_mat.at<double>(0));

        if (dd > line_dist) {
            match = matches.match.erase(match);
            continue;
        }

        ++match;

    }

}




void getLineMatchedSURFKeypoints(const cv::Mat img1, std::vector<cv::KeyPoint>& keypoints1,
    const cv::Mat img2, std::vector<cv::KeyPoint>& keypoints2, const cv::Mat fund) {
    cv::Mat feature_mask;
    getFeatureExtractionRegion(img1, feature_mask);

    int minHessian = 600;
    cv::Ptr<cv::AKAZE> detector = cv::AKAZE::create();

    std::vector<cv::KeyPoint> points1, points2;
    cv::Mat descriptors1, descriptors2;
    detector->detectAndCompute(img1, feature_mask, points1, descriptors1);
    detector->detectAndCompute(img2, feature_mask, points2, descriptors2);

    std::cout << "lpoints1: " << points1.size() << std::endl;
    std::cout << "lpoints2: " << points2.size() << std::endl;

    std::cout << "Descriptor1: " << descriptors1.size() << std::endl;
    std::cout << "Descriptor2: " << descriptors2.size() << std::endl;

    cv::Ptr<cv::DescriptorMatcher> matcher = cv::DescriptorMatcher::create(cv::DescriptorMatcher::BRUTEFORCE_HAMMING);

    std::vector<cv::DMatch> good_matches;
    std::cout << "Create mask ...." << std::endl;
    cv::Mat mask = cv::Mat::zeros(points1.size(), points2.size(), CV_8UC1);

    cv::Mat points1v(points1.size(), 3, CV_64F);
    cv::Mat points2v(3, points2.size(), CV_64F);
    for (int i = 0; i < points1.size(); ++i) {
        points1v.at<double>(i, 0) = points1[i].pt.x;
        points1v.at<double>(i, 1) = points1[i].pt.y;
        points1v.at<double>(i, 2) = 1.0;
    }
    for (int i = 0; i < points2.size(); ++i) {
        points2v.at<double>(0, i) = points2[i].pt.x;
        points2v.at<double>(1, i) = points2[i].pt.y;
        points2v.at<double>(2, i) = 1.0;
    }

    cv::Mat kp_lines21 = fund * points2v;
    std::cout << "points1v = " << points1v.rows << ", " << points1v.cols << std::endl;
    std::cout << "points2v = " << points2v.rows << ", " << points2v.cols << std::endl;
    std::cout << "kp_lines21 = " << kp_lines21.size() << std::endl;

    cv::Mat mask1 = cv::abs(points1v * kp_lines21);
    cv::threshold(mask1, mask1, 0.01, 1, cv::THRESH_BINARY_INV);
    std::cout << "mask1 = " << mask1.rows << ", " << mask1.cols << std::endl;

    std::vector<std::vector<cv::DMatch> > knnMatches;
    std::cout << "knnMatch ..." << std::endl;
    cv::Mat mask2;
    mask1.convertTo(mask2, CV_8UC1);
    matcher->knnMatch(descriptors1, descriptors2, knnMatches, 2, mask2);

    const float ratio_thresh = 0.4f; //0.6
    bool good_match = false;
    for (int m = 0; m < knnMatches.size(); ++m) {
        if (knnMatches[m].size() < 2) continue;
        if (knnMatches[m][0].distance < ratio_thresh * knnMatches[m][1].distance) {
            good_match = true;
            good_matches.push_back(knnMatches[m][0]);
        }
    }

    std::cout << "lgood_matches.size = " << good_matches.size() << std::endl;


    for (size_t i = 0; i < good_matches.size(); ++i) {
        keypoints1.push_back(points1[good_matches[i].queryIdx]);
        keypoints2.push_back(points2[good_matches[i].trainIdx]);
    }

}



//void getMatchedSURFKeypoints(const cv::Mat img1, std::vector<cv::KeyPoint>& keypoints1,
//    const cv::Mat img2, std::vector<cv::KeyPoint>& keypoints2, const cv::Mat fund
//) {
//
//    cv::Mat mask;
//    getFeatureExtractionRegion(img1, mask);
//
//    // Step 1:: Detect
//    int minHessian = 400;
//    cv::Ptr<cv::xfeatures2d::SURF> detector = cv::xfeatures2d::SURF::create(minHessian);
//    std::vector<cv::KeyPoint> points1, points2;
//    cv::Mat descriptors1, descriptors2;
//    detector->detectAndCompute(img1, mask, points1, descriptors1);
//    detector->detectAndCompute(img2, mask, points2, descriptors2);
//
//    std::cout << "points1: " << points1.size() << std::endl;
//    std::cout << "points2: " << points2.size() << std::endl;
//
//
//
//    // Step 2: Match
//    cv::Ptr<cv::DescriptorMatcher> matcher = cv::DescriptorMatcher::create(cv::DescriptorMatcher::FLANNBASED);
//    std::vector<std::vector<cv::DMatch> > knnMatches;
//    matcher->knnMatch(descriptors1, descriptors2, knnMatches, 2);
//    std::cout << "knnMatches.size = " << knnMatches.size() << std::endl;
//
//    const float ratio_thresh = 0.9f;
//    std::vector<cv::DMatch> good_matches;
//    for (size_t i = 0; i < knnMatches.size(); ++i) {
//
//        if (knnMatches[i][0].distance < ratio_thresh * knnMatches[i][1].distance) {
//            good_matches.push_back(knnMatches[i][0]);
//        }
//    }
//    std::cout << "good_matches.size = " << good_matches.size() << std::endl;
//
//    std::vector<cv::DMatch> best_matches;
//    if (!fund.empty()) {
//        std::cout << "CHECK FUND MATRIX CONSTRAINT!!!\n";
//        for (size_t i = 0; i < good_matches.size(); ++i) {
//            cv::DMatch match = good_matches[i];
//            cv::Mat p1 = cv::Mat1d({ points1[match.queryIdx].pt.x, points1[match.queryIdx].pt.y, 1.0 });
//            cv::Mat p2 = cv::Mat1d({ points2[match.trainIdx].pt.x, points2[match.trainIdx].pt.y, 1.0 });
//            cv::Mat pres = p1.t() * fund * p2;
//            if (pres.at<double>() < 0.01) {
//                best_matches.push_back(match);
//                keypoints1.push_back(points1[match.queryIdx]);
//                keypoints2.push_back(points2[match.trainIdx]);
//            }
//            else {
//                // std::cout << i << ": good_match res (SKIP) = " << pres << std::endl;
//            }
//        }
//
//    }
//    else {
//        for (size_t i = 0; i < good_matches.size(); ++i) {
//            keypoints1.push_back(points1[good_matches[i].queryIdx]);
//            keypoints2.push_back(points2[good_matches[i].trainIdx]);
//        }
//    }
//
//
//}



void triangulatePoints(const CameraSystemIntrinsics& intr1, const ImageData& img_data1, const std::vector<cv::Point2f>& points1,
    const CameraSystemIntrinsics& intr2, const ImageData& img_data2, const std::vector<cv::Point2f>& points2, cv::Mat& points3d
) {

    CameraInfo camera_info1, camera_info2;

    camera_info1.intr = intr1;
    camera_info1.rotation_angles[0] = img_data1.coords[0];
    camera_info1.rotation_angles[1] = img_data1.coords[1];
    camera_info1.rotation_angles[2] = img_data1.coords[2];
    camera_info1.translation[0] = img_data1.coords[3];
    camera_info1.translation[1] = img_data1.coords[4];
    camera_info1.translation[2] = img_data1.coords[5];

    camera_info2.intr = intr2;
    camera_info2.rotation_angles[0] = img_data2.coords[0];
    camera_info2.rotation_angles[1] = img_data2.coords[1];
    camera_info2.rotation_angles[2] = img_data2.coords[2];
    camera_info2.translation[0] = img_data2.coords[3];
    camera_info2.translation[1] = img_data2.coords[4];
    camera_info2.translation[2] = img_data2.coords[5];

    triangulatePoints(camera_info1, points1, camera_info2, points2, points3d);


}

cv::Mat getProjMatrix(const CameraInfo& camera_info) {
    glm::dmat3 r = getRotation(camera_info.rotation_angles[0], camera_info.rotation_angles[1], camera_info.rotation_angles[2]);

    glm::dmat3 k = camera_info.intr.getCameraMatrix();
    glm::dvec3 t = camera_info.translation;

    glm::dmat4x3 proj(1.0);
    proj[3] -= t;
    proj = k * glm::transpose(r) * proj;

    cv::Mat mat_proj(3, 4, CV_64F);
    for (size_t col = 0; col < 4; ++col) {
        for (size_t row = 0; row < 3; ++row) {
            mat_proj.at<double>(row, col) = proj[col][row];
        }
    }
    return mat_proj;
}

cv::Mat getRotationTranslationTransform(const CameraInfo& camera_info) {
    glm::dmat3 r = getRotation(camera_info.rotation_angles[0],
        camera_info.rotation_angles[1],
        camera_info.rotation_angles[2]);
    glm::dvec3 t = camera_info.translation;

    glm::dmat4x3 proj(1.0);
    proj[3] -= t;
    proj = glm::transpose(r) * proj;

    cv::Mat mat_rt(3, 4, CV_64F);
    for (size_t col = 0; col < 4; ++col) {
        for (size_t row = 0; row < 3; ++row) {
            mat_rt.at<double>(row, col) = proj[col][row];
        }
    }
    return mat_rt;
}



void triangulatePoints(const CameraInfo& camera_info1, const std::vector<cv::Point2f>& points1,
    const CameraInfo& camera_info2, const std::vector<cv::Point2f>& points2,
    cv::Mat& points3d) {

    cv::Mat mat_proj1 = getProjMatrix(camera_info1);
    cv::Mat mat_proj2 = getProjMatrix(camera_info2);

    cv::Mat points4dh;
    cv::triangulatePoints(mat_proj1, mat_proj2, points1, points2, points4dh);

    cv::convertPointsFromHomogeneous(points4dh.t(), points3d);

}

std::vector<double> getReprojectionErrors(const std::vector<cv::Point2f>& points, const cv::Mat& proj, const cv::Mat& points3d) {

    std::vector<double> errs(points3d.rows);

    cv::Mat points3dh;
    points3d.convertTo(points3dh, CV_64F);
    cv::convertPointsToHomogeneous(points3dh, points3dh);
    points3dh = points3dh.reshape(1);
    cv::Mat ph = proj * points3dh.t();
    cv::convertPointsFromHomogeneous(ph.t(), ph);

    for (size_t i = 0; i < points.size(); ++i) {
        double dx, dy;
        double err;
        dx = abs(ph.at<double>(i, 0) - points[i].x);
        dy = abs(ph.at<double>(i, 1) - points[i].y);
        err = sqrt(dx * dx + dy * dy);
        errs[i] = err;
    }

    return errs;

}

double getReprojectionError(const Map3D& map, const std::vector<CameraInfo>& cameras, const std::vector<Features>& features) {

    double err = 0.0;

    for (size_t i = 0; i < map.size(); ++i) {
        err += getReprojectionError(map[i], cameras, features);

    }

    return err;

}

std::vector<double> getReprojectionErrors(
    const Map3D& map,
    const std::vector<CameraInfo>& cameras,
    const std::vector<Features>& features
) {

    std::vector<double> errs(map.size());
    for (size_t i = 0; i < map.size(); ++i) {
        errs[i] = getReprojectionError(map[i], cameras, features);
        errs[i] /= map[i].views.size();
    }
    return errs;

}


double getReprojectionError(const WorldPoint3D& point3d,
    const std::vector<CameraInfo>& cameras,
    const std::vector<Features>& features
) {
    double err = 0.0;
    for (auto& view : point3d.views) {
        cv::Mat proj = getProjMatrix(cameras[view.first]);
        const cv::Point2f& point = features[view.first].keypoints[view.second].pt;
        cv::Matx41d point3dh(
            point3d.pt.x, point3d.pt.y, point3d.pt.z, 1.0
        );
        cv::Mat pred = proj * cv::Mat(point3dh);
        pred = pred / pred.at<double>(2);
        double dx = pred.at<double>(0) - double(point.x);
        double dy = pred.at<double>(1) - double(point.y);
        err += 0.5 * (dx * dx + dy * dy);
    }
    return err;
}



std::vector<double> getZDistanceFromCamera(const CameraInfo& camera_info,
    const cv::Mat& points3d) {

    std::vector<double> zdist(points3d.rows);

    cv::Mat rt1 = getRotationTranslationTransform(camera_info);
    cv::Mat points3dh;
    points3d.convertTo(points3dh, CV_64F);
    cv::convertPointsToHomogeneous(points3dh, points3dh);
    points3dh = points3dh.reshape(1);
    cv::Mat pc = rt1 * points3dh.t();

    for (size_t i = 0; i < pc.cols; ++i) {
        zdist[i] = pc.at<double>(2, i);
    }

    return zdist;

}

void removeOutliersByError(Map3D& map,
    const std::vector<CameraInfo>& cameras,
    const std::vector<Features>& features,
    const float percentile) {

    std::vector<double> errs = getReprojectionErrors(map, cameras, features);

    double min_err = (*std::min_element(errs.begin(), errs.end()));
    double max_err = (*std::max_element(errs.begin(), errs.end()));
    double bound_error = min_err + (max_err - min_err) * (1 - percentile);

    auto w = 0;
    auto wit = map.begin();
    auto r = 0;
    while (r < map.size()) {
        if (errs[r] < bound_error) {
            if (r != w) {
                std::swap(map[r], map[w]);
            }
            ++w;
            ++wit;
        }
        else {
            std::cout << "-- erase " << errs[r] << ": " << map[r] << std::endl;
        }
        ++r;
    }

    map.erase(wit, map.end());

}

Map3D reduceMapByError(const Map3D& map,
    const std::vector<CameraInfo>& cameras,
    const std::vector<Features>& features,
    const float ratio) {

    if (ratio == 1.0) return Map3D(map);


    std::vector<double> errs = getReprojectionErrors(map, cameras, features);

    typedef std::pair<int, double> ErrEl;
    std::vector<ErrEl> errsi(errs.size());
    for (int i = 0; i < errs.size(); ++i) {
        errsi[i] = std::make_pair(i, errs[i]);
    }

    std::sort(errsi.begin(), errsi.end(), [](const ErrEl& a, const ErrEl& b) {
        return a.second < b.second;
        });

    int new_size = map.size() * ratio;
    Map3D mapr(new_size);

    for (int i = 0; i < new_size; ++i) {
        mapr[i] = map[errsi[i].first];
    }

    std::cout << "Reduced map: from.size = " << map.size()
        << " to.size = " << mapr.size()
        << " (ratio: " << ratio << ")"
        << std::endl;

    return mapr;

}

std::int32_t getNextBestView(const Map3D& map,
    const std::unordered_set<std::int32_t>& views,
    DSU<std::pair<std::int32_t, std::int32_t> >& ccomp,
    const std::vector<Matches>& image_matches,
    const std::map<std::pair<std::int32_t, std::int32_t>, std::int32_t>& matches_index
) {

    std::int32_t view_id = -1;
    std::int32_t match_cnt = 0;

    std::int32_t view_id_size = -1;
    std::int32_t match_size = 0;
    for (auto it = views.begin(); it != views.end(); ++it) {
        std::int32_t view = (*it);
        std::int32_t cnt = 0;
        for (auto& wp : map) {

            bool point_match = false;
            for (auto& wp_view : wp.views) {
                std::pair<std::int32_t, std::int32_t> v = wp_view;
                std::pair<std::int32_t, std::int32_t> m_ind = std::make_pair(v.first, view);
                auto m = matches_index.find(m_ind);
                if (m == matches_index.end()) {
                    // std::cout << "skip pair = " << m_ind.first << ", "  
                    //           << m_ind.second << std::endl;
                    continue;
                }

                if (match_size < image_matches[m->second].match.size()) {
                    view_id_size = view;
                    match_size = image_matches[m->second].match.size();
                }


                for (size_t i = 0; i < image_matches[m->second].match.size(); ++i) {
                    const cv::DMatch& match = image_matches[m->second].match[i];
                    std::int32_t p_id;
                    if (image_matches[m->second].image_index.first == view) {
                        p_id = match.queryIdx;
                    }
                    else {
                        p_id = match.trainIdx;
                    }
                    std::pair<std::int32_t, std::int32_t> p = std::make_pair(view, p_id);
                    if (ccomp.Connected(v, p)) {
                        point_match = true;
                        break;
                    }
                }

                if (point_match) {
                    ++cnt;
                    break;
                }

            }


        }
        std::cout << "view = " << view << ", cnt = " << cnt
            << ", match_size = " << match_size
            << std::endl;
        if (match_cnt < cnt) {
            view_id = view;
            match_cnt = cnt;
        }
    }

    if (view_id < 0) {
        view_id = view_id_size;
    }

    return view_id;

}


std::int32_t getNextBestViewByViews(const Map3D& map,
    const std::unordered_set<std::int32_t>& todo_views,
    const std::unordered_set<std::int32_t>& used_views,
    const std::vector<Matches>& image_matches,
    const std::map<std::pair<std::int32_t, std::int32_t>, std::int32_t>& matches_index
) {

    std::int32_t view_id = -1;
    std::int32_t max_match_cnt = 0;
    for (auto it = todo_views.begin(); it != todo_views.end(); ++it) {
        std::int32_t view = (*it);
        std::int32_t match_sum = 0;
        for (auto itc = used_views.begin(); itc != used_views.end(); ++itc) {
            std::int32_t used_view = (*itc);
            std::pair<std::int32_t, std::int32_t> m_ind = std::make_pair(view, used_view);
            auto m = matches_index.find(m_ind);
            if (m == matches_index.end()) {
                continue;
            }
            match_sum += image_matches[m->second].match.size();

        }
        if (max_match_cnt < match_sum) {
            view_id = view;
            max_match_cnt = match_sum;

        }
    }
    return view_id;
}

void mergeToTheMap(Map3D& map,
    const Map3D& local_map,
    DSU<std::pair<std::int32_t, std::int32_t> >& ccomp
) {

    using namespace std::chrono;

    auto t0 = high_resolution_clock::now();

    Map3D map_temp;

    std::int32_t connect_cnt = 0;
    std::int32_t idx = 0;
    std::cout << std::endl;
    for (auto lp : local_map) {
        std::pair<std::int32_t, std::int32_t> lp_view = (*lp.views.begin());
        // std::cout << idx << " : lp = " << lp;

        std::int32_t cnt = 0;
        bool skip = false;
        double min_dist = 10000.0;
        std::int32_t min_dist_id = -1;
        std::int32_t looked_points = 0;
        for (std::int32_t i = 0; i < map.size(); ++i) {
            WorldPoint3D& wp = map[i];
            std::pair<std::int32_t, std::int32_t> wp_view = (*wp.views.begin());

            if (ccomp.Connected(lp_view, wp_view)) {
                double dist = cv::norm(lp.pt - wp.pt);
                ++looked_points;
                if (dist < min_dist) {
                    min_dist = dist;
                    min_dist_id = i;
                }
            }
        }

        if (min_dist_id >= 0) {

            skip = true;


            if (min_dist < 20.0) {
                WorldPoint3D& wp = map[min_dist_id];
                for (auto view : lp.views) {
                    wp.views.insert(view);
                }
                skip = true;
                ++connect_cnt;
            }
            else {
                // std::cout << idx << "] DISCARD!!!! min_dist = " << min_dist << std::endl;
            }

        }

        if (/*cnt == 0 && */ !skip) {
            map_temp.push_back(lp);
        }
        else {
            // std::cout << idx << "] skip point\n";
        }


        ++idx;
    }

    for (auto& mt : map_temp) {
        map.push_back(mt);
    }

    auto t1 = high_resolution_clock::now();
    auto dur = duration_cast<microseconds>(t1 - t0);
    std::cout << ", merge_connected = " << connect_cnt << ","
        << ", merge_time = " << dur.count() / 1e+6;
}

void mergeToTheMapImproved(Map3D& map,
    const Map3D& local_map,
    DSU<std::pair<std::int32_t, std::int32_t> >& ccomp
) {
    using namespace std::chrono;

    auto t0 = high_resolution_clock::now();

    auto world_point_comp = [](const WorldPoint3D& wp1,
        const WorldPoint3D& wp2) {
            return wp1.component_id < wp2.component_id;
        };

    auto world_point_val_comp = [](const WorldPoint3D& wp1,
        const int& v) {
            return wp1.component_id < v;
        };

    std::sort(map.begin(), map.end(), world_point_comp);


    auto el = std::lower_bound(map.begin(), map.end(), 1976, world_point_val_comp);

    Map3D map_temp;


    std::int32_t connect_cnt = 0;
    std::int32_t idx = 0;
    std::int32_t cnt = 0;
    for (auto lp : local_map) {
        double min_dist = 10000.0;
        std::int32_t looked_points = 0;
        std::vector<WorldPoint3D>::iterator min_wp_it = map.end();
        std::pair<std::int32_t, std::int32_t> lp_view = (*lp.views.begin());
        std::int32_t comp_id = ccomp.find(lp_view);
        auto first_el = map.begin();
        auto el = std::lower_bound(first_el, map.end(), comp_id, world_point_val_comp);

        while (el != map.end() && el->component_id == comp_id) {
            double dist = cv::norm(lp.pt - el->pt);
            ++looked_points;

            if (dist < min_dist) {
                min_dist = dist;
                min_wp_it = el;
            }

            el = std::lower_bound(std::next(el), map.end(), comp_id,
                world_point_val_comp);
        }

        if (min_wp_it != map.end()) {
            if (min_dist < 20.0) {
                WorldPoint3D& wp = (*min_wp_it);
                for (auto view : lp.views) {
                    wp.views.insert(view);
                }
                ++connect_cnt;
            }
        }
        else {
            map_temp.push_back(lp);
        }

        ++cnt;
    }

    // Copy from temp 
    for (auto& mt : map_temp) {
        map.push_back(mt);
    }

    auto t1 = high_resolution_clock::now();
    auto dur = duration_cast<microseconds>(t1 - t0);
    std::cout << ", merge_improved_connected = " << connect_cnt << ","
        << ", merge_improved_time = " << dur.count() / 1e+6;

}

void combineMapComponents(Map3D& map, const double max_keep_dist) {
    auto world_point_comp = [](const WorldPoint3D& wp1,
        const WorldPoint3D& wp2) {
            return wp1.component_id != wp2.component_id
                ? wp1.component_id < wp2.component_id
                : cv::norm(wp1.pt) < cv::norm(wp2.pt);
        };
    std::sort(map.begin(), map.end(), world_point_comp);

    auto first = map.begin();
    auto second = std::next(first);
    auto w = map.begin();
    bool discard_first = false;
    while (second != map.end()) {
        discard_first = false;
        if (first->component_id == second->component_id) {
            double dist = cv::norm(first->pt - second->pt);
            if (dist < max_keep_dist) {
                first->pt = (first->pt + second->pt) * 0.5;
                for (auto view : second->views) {
                    first->views.insert(view);
                }
                ++second;
            }
            else {
                int discard_id = first->component_id;
                while (first->component_id == discard_id && second != map.end()) {
                    first = second;
                    second = std::next(first);
                }
                if (first->component_id == discard_id) {
                    discard_first = true;
                }
            }
        }
        else {
            // keep
            if (first != w) {
                *w = std::move(*first);
            }
            first = second;
            second = std::next(first);
            ++w;
        }
    }

    if (!discard_first) {
        if (first != w) {
            *w = std::move(*first);
        }
        ++w;
    }

    map.erase(w, map.end());

}

void mergeAndCombinePoints(Map3D& map,
    const Map3D& local_map,
    const double max_keep_dist
) {

    using namespace std::chrono;
    auto t0 = high_resolution_clock::now();

    // std::cout << "\nMerge AND Combine Points:\n";
    map.insert(map.end(), local_map.begin(), local_map.end());
    combineMapComponents(map, max_keep_dist);

    auto t1 = high_resolution_clock::now();
    auto dur = duration_cast<microseconds>(t1 - t0);
    std::cout << ", merge_combine_time = " << dur.count() / 1e+6;
}

void getKeyPointColors(const cv::Mat& img,
    const cv::KeyPoint& point,
    Point3DColor& p3d,
    const bool add_colors,
    double dangle,
    double image_scale
) {
    if (add_colors) {
        p3d.color += getGlmColorFromImage(img, point.pt, image_scale);
    }
    else {
        p3d.color = getGlmColorFromImage(img, point.pt, image_scale);
    }

    cv::Point2f tl_pt = point.pt + cv::Point2f(
        point.size * cos(deg2rad(135.0 + dangle)),
        point.size * sin(deg2rad(135 + dangle)));
    if (add_colors) {
        p3d.color_tl += getGlmColorFromImage(img, tl_pt, image_scale);
    }
    else {
        p3d.color_tl = getGlmColorFromImage(img, tl_pt, image_scale);
    }

    cv::Point2f tr_pt = point.pt + cv::Point2f(
        point.size * cos(deg2rad(45.0 + dangle)),
        point.size * sin(deg2rad(45.0 + dangle)));
    if (add_colors) {
        p3d.color_tr += getGlmColorFromImage(img, tr_pt, image_scale);
    }
    else {
        p3d.color_tr = getGlmColorFromImage(img, tr_pt, image_scale);
    }

    cv::Point2f bl_pt = point.pt + cv::Point2f(
        point.size * cos(deg2rad(225.0 + dangle)),
        point.size * sin(deg2rad(225.0 + dangle)));
    if (add_colors) {
        p3d.color_bl += getGlmColorFromImage(img, bl_pt, image_scale);
    }
    else {
        p3d.color_bl = getGlmColorFromImage(img, bl_pt, image_scale);
    }

    cv::Point2f br_pt = point.pt + cv::Point2f(
        point.size * cos(deg2rad(315.0 + dangle)),
        point.size * sin(deg2rad(315.0 + dangle)));
    if (add_colors) {
        p3d.color_br += getGlmColorFromImage(img, br_pt, image_scale);
    }
    else {
        p3d.color_br = getGlmColorFromImage(img, br_pt, image_scale);
    }

}



std::ostream& operator<<(std::ostream& os, const WorldPoint3D& wp) {
    os << "(" << wp.pt << ") from ";
    bool first = true;
    for (auto& v : wp.views) {
        if (!first) {
            os << ", ";
        }
        else {
            first = false;
        }
        os << "[" << v.first << "," << v.second << "]";
    }
    os << ", comp_id = " << wp.component_id;
    return os;
}

std::ostream& operator<<(std::ostream& os, const ImagePair& ip) {
    os << "(" << ip.first << ", " << ip.second << ")";
    return os;
}

// =====================================================
// ======== Ceres / BundleOptimization =================

//struct ReprojectionErrorFunctor {
//    ReprojectionErrorFunctor(const CameraInfo& cam_info, const cv::Point2f& img_point)
//        : camera_info(cam_info), image_point(img_point) {
//
//        proj = getProjMatrix(camera_info);
//    }
//
//    template<typename T>
//    bool operator()(const T* const point3d,
//        T* residuals) const {
//        T p[3];
//        p[0] = proj.at<double>(0, 0) * point3d[0] +
//            proj.at<double>(0, 1) * point3d[1] +
//            proj.at<double>(0, 2) * point3d[2] +
//            proj.at<double>(0, 3);
//        p[1] = proj.at<double>(1, 0) * point3d[0] +
//            proj.at<double>(1, 1) * point3d[1] +
//            proj.at<double>(1, 2) * point3d[2] +
//            proj.at<double>(1, 3);
//        p[2] = proj.at<double>(2, 0) * point3d[0] +
//            proj.at<double>(2, 1) * point3d[1] +
//            proj.at<double>(2, 2) * point3d[2] +
//            proj.at<double>(2, 3);
//        // std::cout << "p0/p2 = " << p[0] / p[2] << std::endl;
//        // std::cout << "p1/p2 = " << p[1] / p[2] << std::endl;
//        residuals[0] = p[0] / p[2] - T(image_point.x);
//        residuals[1] = p[1] / p[2] - T(image_point.y);
//        // std::cout << "res0 = " << residuals[0] << std::endl;
//        // std::cout << "res1 = " << residuals[1] << std::endl;
//        return true;
//    }
//
//    static ceres::CostFunction* Create(const CameraInfo& cam_info, const cv::Point2f& img_point) {
//        return (new ceres::AutoDiffCostFunction<ReprojectionErrorFunctor, 2, 3>(
//            new ReprojectionErrorFunctor(cam_info, img_point)));
//    }
//
//    const CameraInfo& camera_info;
//    const cv::Point2f& image_point;
//    cv::Mat proj;
//};
//
//
//void OptimizeBundle(Map3D& map, const std::vector<CameraInfo>& cameras, const std::vector<Features>& features) {
//
//    // TEST output
//    // double R[2];
//    // ReprojectionErrorFunctor* f = new ReprojectionErrorFunctor(cameras[2], features[2].keypoints[map[0].views[2]].pt);
//    // ReprojectionErrorFunctor* f2 = new ReprojectionErrorFunctor(cameras[5], features[5].keypoints[map[0].views[5]].pt);
//    // ReprojectionErrorFunctor* f3 = new ReprojectionErrorFunctor(cameras[5], features[5].keypoints[map[1].views[5]].pt);
//    // ReprojectionErrorFunctor* f4 = new ReprojectionErrorFunctor(cameras[5], features[5].keypoints[map[3].views[5]].pt);
//
//
//    // std::cout << "Optimize Bundle!\n";
//
//    ceres::Problem problem;
//
//
//    double* points = new double[3 * map.size()];
//    for (size_t i = 0; i < map.size(); ++i) {
//        points[3 * i + 0] = map[i].pt.x;
//        points[3 * i + 1] = map[i].pt.y;
//        points[3 * i + 2] = map[i].pt.z;
//    }
//
//    for (size_t i = 0; i < map.size(); ++i) {
//        for (auto& view : map[i].views) {
//            ceres::CostFunction* cost_function = ReprojectionErrorFunctor::Create(
//                cameras[view.first],
//                features[view.first].keypoints[view.second].pt);
//            problem.AddResidualBlock(cost_function,
//                NULL,
//                // new ceres::CauchyLoss(0.5),
//                &points[3 * i]);
//        }
//    }
//
//    // DEBUG OUTPUT
//    // std::cout << "ERRORS BEFORE: \n";
//    // (*f)(points, R);
//    // std::cout << "Residuals: " << R[0] << ", " << R[1] << std::endl;
//    // (*f2)(points, R);
//    // std::cout << "Residuals2: " << R[0] << ", " << R[1] << std::endl;
//    // (*f3)(&points[3], R);
//    // std::cout << "Residuals3: " << R[0] << ", " << R[1] << std::endl;
//    // (*f4)(&points[9], R);
//    // std::cout << "Residuals4: " << R[0] << ", " << R[1] << std::endl;
//    // std::cout << "points[0-2]: " << points[0] << ", " << points[1] << ", " << points[2] << std::endl;
//
//    // Make Ceres automatically detect the bundle structure. Note that the
//    // standard solver, SPARSE_NORMAL_CHOLESKY, also works fine but it is slower
//    // for standard bundle adjustment problems.
//    ceres::Solver::Options options;
//    options.linear_solver_type = ceres::DENSE_SCHUR;
//    options.minimizer_progress_to_stdout = true;
//    options.max_num_iterations = 500;
//    options.eta = 1e-2;
//    options.max_solver_time_in_seconds = 3500;
//    options.logging_type = ceres::LoggingType::SILENT;
//    ceres::Solver::Summary summary;
//    ceres::Solve(options, &problem, &summary);
//    // std::cout << std::endl;
//    std::cout << summary.BriefReport();
//    // std::cout << summary.FullReport();
//    std::cout << ", SolverTime = " << summary.total_time_in_seconds;
//
//
//    if (not (summary.termination_type == ceres::CONVERGENCE)) {
//        std::cerr << "Bundle adjustment failed." << std::endl;
//        std::cout << summary.FullReport();
//        return;
//    }
//
//
//    // DEBUG OUTPUT
//    // std::cout << "ERRORS AFTER: \n";
//    // (*f)(points, R);
//    // std::cout << "Residuals: " << R[0] << ", " << R[1] << std::endl;
//    // (*f2)(points, R);
//    // std::cout << "Residuals2: " << R[0] << ", " << R[1] << std::endl;
//    // (*f3)(&points[3], R);
//    // std::cout << "Residuals3: " << R[0] << ", " << R[1] << std::endl;
//    // (*f4)(&points[9], R);
//    // std::cout << "Residuals4: " << R[0] << ", " << R[1] << std::endl;
//    // std::cout << "points[0-2]: " << points[0] << ", " << points[1] << ", " << points[2] << std::endl;
//
//
//    // === Copy points back ===
//    for (size_t i = 0; i < map.size(); ++i) {
//        map[i].pt.x = points[3 * i];
//        map[i].pt.y = points[3 * i + 1];
//        map[i].pt.z = points[3 * i + 2];
//    }
//
//
//    delete[] points;
//
//}
//

//
SFM_NS_E