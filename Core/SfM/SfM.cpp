#include "SfM.hpp"

#include <cassert>

#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>

SFM_NS_B

SfM3D::SfM3D(const std::vector<cv::Mat>& intrinsics) {
    cameras_.resize(intrinsics.size());
    for (size_t i = 0; i < intrinsics.size(); ++i) {
        cameras_[i].K = intrinsics[i].clone();
        cameras_[i].pose.registered = false;
    }
}

void SfM3D::addImages(const std::vector<cv::Mat>& images) {
    images_ = images;
    features_.resize(images.size());

    if (cameras_.size() != images_.size()) {
        throw std::runtime_error("Number of cameras must match number of images");
    }
}

void SfM3D::extractFeatures() {
    cv::Ptr<cv::SIFT> sift = cv::SIFT::create();
    for (size_t i = 0; i < images_.size(); ++i) {
        sift->detectAndCompute(
            images_[i],
            cv::noArray(),
            features_[i].keypoints,
            features_[i].descriptors
        );
    }
}

void SfM3D::matchFeatures() {
    cv::BFMatcher matcher(cv::NORM_L2);

    for (int i = 0; i < (int)images_.size(); ++i) {
        if (features_[i].descriptors.empty())
            continue;

        for (int j = i + 1; j < (int)images_.size(); ++j) {
            if (features_[j].descriptors.empty())
                continue;

            Matches m;
            m.img1 = i;
            m.img2 = j;

            matcher.match(
                features_[i].descriptors,
                features_[j].descriptors,
                m.matches
            );

            matches_.push_back(std::move(m));
        }
    }
}

void SfM3D::buildTracks() {
    for (const auto& m : matches_) {
        for (const auto& d : m.matches) {
            Obs o1{ m.img1, d.queryIdx };
            Obs o2{ m.img2, d.trainIdx };
            tracks_.union_rel(o1, o2);
        }
    }
}

void SfM3D::reconstruct() {
    buildTracks();
    bootstrap();

    while (true) {
        int next = selectNextView();
        if (next < 0)
            break;

        registerNextView(next);

        // triangulate new tracks with existing registered views
        for (int v : registered_views_) {
            if (v == next) continue;
            triangulateTracks(v, next);
        }
    }
}

void SfM3D::bootstrap() {
    std::cout << "Starting bootstrap\n";
    for (const auto& m : matches_) {
        cv::Mat R, t;
        //std::cout << "getting common tracks\n";
        std::vector<int> common_tracks = getCommonTracks(m.img1, m.img2);

        //if (common_tracks.size() < 100)
        //    continue;

        if (!estimateInitialPose(m.img1, m.img2, R, t))
            continue;

        //std::cout << "registering camera\n";
         //register first camera at origin
        cameras_[m.img1].pose.R = cv::Mat::eye(3, 3, CV_64F);
        cameras_[m.img1].pose.t = cv::Mat::zeros(3, 1, CV_64F);
        cameras_[m.img1].pose.registered = true;

        //// second camera
        cameras_[m.img2].pose.R = R;
        cameras_[m.img2].pose.t = t;
        cameras_[m.img2].pose.registered = true;

        registered_views_.insert(m.img1);
        registered_views_.insert(m.img2);

        //std::cout << "Triangulating tracks\n";
        triangulateTracks(m.img1, m.img2);
        return;
    }

    throw std::runtime_error("SfM bootstrap failed");
}

bool SfM3D::estimateInitialPose(
    int img1, int img2,
    cv::Mat& R, cv::Mat& t
) {

    std::vector<cv::Point2f> pts1, pts2;

    for (const auto& m : matches_) {
        if (m.img1 == img1 && m.img2 == img2) {
            for (const auto& d : m.matches) {
                pts1.push_back(features_[img1].keypoints[d.queryIdx].pt);
                pts2.push_back(features_[img2].keypoints[d.trainIdx].pt);
            }
            break;
        }
    }

    if (pts1.size() < 100)
        return false;

    cv::Mat mask;
    cv::Mat E = cv::findEssentialMat(
        pts1, pts2,
        cameras_[img1].K,
        cv::RANSAC, 0.999, 1.0, mask
    );

    if (E.empty())
        return false;

    cv::recoverPose(
        E, pts1, pts2,
        cameras_[img1].K, R, t, mask
    );

    return true;
}

std::vector<int> SfM3D::getCommonTracks(int i, int j)  {
    std::vector<int> common;
    auto comps = tracks_.getComponentIds();

    for (int cid : comps) {
        bool has_i = false, has_j = false;
        auto elems = tracks_.getElementsById(cid);
        for (auto& e : elems) {
            if (e.first == i) has_i = true;
            if (e.first == j) has_j = true;
        }
        if (has_i && has_j)
            common.push_back(cid);
    }
    return common;
}

void SfM3D::triangulateTracks(int img1, int img2) {
    auto track_ids = getCommonTracks(img1, img2);

    cv::Mat P1 = cameras_[img1].K *
        (cv::Mat_<double>(3, 4) <<
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0);

    cv::Mat Rt;
    cv::hconcat(cameras_[img2].pose.R,
        cameras_[img2].pose.t, Rt);
    cv::Mat P2 = cameras_[img2].K * Rt;

    for (int tid : track_ids) {
        auto obs = tracks_.getElementsById(tid);

        int kp1 = -1, kp2 = -1;
        for (auto& o : obs) {
            if (o.first == img1) kp1 = o.second;
            if (o.first == img2) kp2 = o.second;
        }

        if (kp1 < 0 || kp2 < 0)
            continue;

        cv::Point2f p1 = features_[img1].keypoints[kp1].pt;
        cv::Point2f p2 = features_[img2].keypoints[kp2].pt;

        cv::Mat X;
        cv::triangulatePoints(
            P1, P2,
            std::vector<cv::Point2f>{p1},
            std::vector<cv::Point2f>{p2},
            X
        );

        X /= X.at<float>(3);

        WorldPoint3D wp;
        wp.xyz = cv::Point3d(
            X.at<float>(0),
            X.at<float>(1),
            X.at<float>(2)
        );
        wp.track_id = tid;

        map_.push_back(wp);
    }
}

int SfM3D::selectNextView() {
    int best_view = -1;
    int best_score = 0;

    for (int i = 0; i < (int)images_.size(); ++i) {
        if (cameras_[i].pose.registered)
            continue;

        int score = 0;
        for (const auto& p : map_) {
            auto obs = tracks_.getElementsById(p.track_id);
            for (auto& o : obs) {
                if (o.first == i) {
                    ++score;
                    break;
                }
            }
        }

        if (score > best_score) {
            best_score = score;
            best_view = i;
        }
    }

    return (best_score >= 30) ? best_view : -1;
}

void SfM3D::registerNextView(int img) {
    std::vector<cv::Point3f> pts3d;
    std::vector<cv::Point2f> pts2d;

    for (const auto& p : map_) {
        auto obs = tracks_.getElementsById(p.track_id);
        for (auto& o : obs) {
            if (o.first == img) {
                pts3d.emplace_back(p.xyz);
                pts2d.push_back(
                    features_[img].keypoints[o.second].pt
                );
            }
        }
    }

    if (pts3d.size() < 30)
        return;

    cv::Mat rvec, tvec;
    cv::solvePnPRansac(
        pts3d, pts2d,
        cameras_[img].K,
        cv::noArray(),
        rvec, tvec
    );

    cv::Rodrigues(rvec, cameras_[img].pose.R);
    cameras_[img].pose.t = tvec;
    cameras_[img].pose.registered = true;
    registered_views_.insert(img);
}

SFM_NS_E