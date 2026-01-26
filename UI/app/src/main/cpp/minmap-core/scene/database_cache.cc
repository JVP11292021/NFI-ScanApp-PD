#include "database_cache.h"

#include "../util/string.h"
#include "../util/timer.h"

namespace colmap {
namespace {

std::vector<Eigen::Vector2d> FeatureKeypointsToPointsVector(
    const FeatureKeypoints& keypoints) {
  std::vector<Eigen::Vector2d> points(keypoints.size());
  for (size_t i = 0; i < keypoints.size(); ++i) {
    points[i] = Eigen::Vector2d(keypoints[i].x, keypoints[i].y);
  }
  return points;
}

}  // namespace

DatabaseCache::DatabaseCache()
    : correspondence_graph_(std::make_shared<class CorrespondenceGraph>()) {}

void DatabaseCache::Load(const Database& database,
                         const size_t min_num_matches,
                         const bool ignore_watermarks,
                         const std::unordered_set<std::string>& image_names) {
  const bool has_rigs = database.NumRigs() > 0;
  const bool has_frames = database.NumFrames() > 0;

  //////////////////////////////////////////////////////////////////////////////
  // Load rigs
  //////////////////////////////////////////////////////////////////////////////

  Timer timer;

  timer.Start();
  LOG(MM_INFO) << "Loading rigs...";

  {
    std::vector<class Rig> rigs = database.ReadAllRigs();
    rigs_.reserve(rigs.size());
    for (auto& rig : rigs) {
      rigs_.emplace(rig.RigId(), std::move(rig));
    }
  }

  LOG(MM_INFO) << StringPrintf(
      " %d in %.3fs", rigs_.size(), timer.ElapsedSeconds());

  //////////////////////////////////////////////////////////////////////////////
  // Load cameras
  //////////////////////////////////////////////////////////////////////////////

  timer.Restart();
  LOG(MM_INFO) << "Loading cameras...";

  {
    std::vector<struct Camera> cameras = database.ReadAllCameras();
    cameras_.reserve(cameras.size());
    for (auto& camera : cameras) {
      if (!has_rigs) {
        // For backwards compatibility with old databases from before having
        // support for rigs/frames, we create a rig for each camera.
        class Rig rig;
        rig.SetRigId(camera.camera_id);
        rig.AddRefSensor(camera.SensorId());
        rigs_.emplace(rig.RigId(), std::move(rig));
      }
      cameras_.emplace(camera.camera_id, std::move(camera));
    }
  }

  LOG(MM_INFO) << StringPrintf(
      " %d in %.3fs", cameras_.size(), timer.ElapsedSeconds());

  //////////////////////////////////////////////////////////////////////////////
  // Load frames
  //////////////////////////////////////////////////////////////////////////////

  timer.Restart();
  LOG(MM_INFO) << "Loading frames...";

  std::unordered_map<image_t, frame_t> image_to_frame_id;

  {
    std::vector<class Frame> frames = database.ReadAllFrames();
    frames_.reserve(frames.size());
    for (auto& frame : frames) {
      for (const auto& data_id : frame.DataIds()) {
        if (data_id.sensor_id.type == SensorType::CAMERA) {
          image_to_frame_id.emplace(data_id.id, frame.FrameId());
        }
      }
      frames_.emplace(frame.FrameId(), std::move(frame));
    }
  }

  LOG(MM_INFO) << StringPrintf(
      " %d in %.3fs", frames_.size(), timer.ElapsedSeconds());

  //////////////////////////////////////////////////////////////////////////////
  // Load matches
  //////////////////////////////////////////////////////////////////////////////

  timer.Restart();
  LOG(MM_INFO) << "Loading matches...";

  const std::vector<std::pair<image_pair_t, TwoViewGeometry>>
      two_view_geometries = database.ReadTwoViewGeometries();

  LOG(MM_INFO) << StringPrintf(
      " %d in %.3fs", two_view_geometries.size(), timer.ElapsedSeconds());

  auto UseInlierMatchesCheck = [min_num_matches, ignore_watermarks](
                                   const TwoViewGeometry& two_view_geometry) {
    return static_cast<size_t>(two_view_geometry.inlier_matches.size()) >=
               min_num_matches &&
           (!ignore_watermarks ||
            two_view_geometry.config != TwoViewGeometry::WATERMARK);
  };

  //////////////////////////////////////////////////////////////////////////////
  // Load images
  //////////////////////////////////////////////////////////////////////////////

  timer.Restart();
  LOG(MM_INFO) << "Loading images...";

  std::unordered_set<frame_t> frame_ids;

  {
    std::vector<class Image> images = database.ReadAllImages();
    const size_t num_images = images.size();

    if (has_frames) {
      for (auto& image : images) {
        image.SetFrameId(image_to_frame_id.at(image.ImageId()));
      }
    } else {
      for (auto& image : images) {
        image_to_frame_id.emplace(image.ImageId(), image.ImageId());
        if (!image_names.empty() && image_names.count(image.Name()) == 0) {
          continue;
        }
        // For backwards compatibility with old databases from before having
        // support for rigs/frames, we create a frame for each image.
        class Frame frame;
        frame.SetFrameId(image.ImageId());
        frame.SetRigId(image.CameraId());
        frame.AddDataId(image.DataId());
        image.SetFrameId(frame.FrameId());
        frames_.emplace(frame.FrameId(), std::move(frame));
      }
    }

    // Determines for which images data should be loaded.
    if (image_names.empty()) {
      for (const auto& image : images) {
        frame_ids.insert(image.FrameId());
      }
    } else {
      for (const auto& image : images) {
        if (image_names.count(image.Name()) > 0) {
          frame_ids.insert(image.FrameId());
        }
      }
    }

    // Collect all images that are connected in the correspondence graph.
    std::unordered_set<frame_t> connected_frame_ids;
    connected_frame_ids.reserve(frame_ids.size());
    for (const auto& [pair_id, two_view_geometry] : two_view_geometries) {
      if (UseInlierMatchesCheck(two_view_geometry)) {
        const auto [image_id1, image_id2] =
            Database::PairIdToImagePair(pair_id);
        const frame_t frame_id1 = image_to_frame_id.at(image_id1);
        const frame_t frame_id2 = image_to_frame_id.at(image_id2);
        if (frame_ids.count(frame_id1) > 0 && frame_ids.count(frame_id2) > 0) {
          connected_frame_ids.insert(frame_id1);
          connected_frame_ids.insert(frame_id2);
        }
      }
    }

    // Remove unconnected frames.
    for (auto it = frames_.begin(); it != frames_.end();) {
      if (connected_frame_ids.count(it->first) == 0) {
        it = frames_.erase(it);
      } else {
        ++it;
      }
    }

    // Load images with correspondences and discard images without
    // correspondences, as those images are useless for SfM.
    images_.reserve(connected_frame_ids.size());
    for (auto& image : images) {
      if (connected_frame_ids.count(image.FrameId()) == 0) {
        continue;
      }

      const image_t image_id = image.ImageId();
      image.SetPoints2D(
          FeatureKeypointsToPointsVector(database.ReadKeypoints(image_id)));
      images_.emplace(image_id, std::move(image));

      if (database.ExistsPosePrior(image_id)) {
        pose_priors_.emplace(image_id, database.ReadPosePrior(image_id));
      }
    }

    LOG(MM_INFO) << StringPrintf(" %d in %.3fs (connected %d)",
                              num_images,
                              timer.ElapsedSeconds(),
                              images_.size());
  }

  //////////////////////////////////////////////////////////////////////////////
  // Build correspondence graph
  //////////////////////////////////////////////////////////////////////////////

  timer.Restart();
  LOG(MM_INFO) << "Building correspondence graph...";

  correspondence_graph_ = std::make_shared<class CorrespondenceGraph>();

  for (const auto& [image_id, image] : images_) {
    correspondence_graph_->AddImage(image_id, image.NumPoints2D());
  }

  size_t num_ignored_image_pairs = 0;
  for (const auto& [pair_id, two_view_geometry] : two_view_geometries) {
    if (UseInlierMatchesCheck(two_view_geometry)) {
      const auto [image_id1, image_id2] = Database::PairIdToImagePair(pair_id);
      const frame_t frame_id1 = image_to_frame_id.at(image_id1);
      const frame_t frame_id2 = image_to_frame_id.at(image_id2);
      if (frame_ids.count(frame_id1) > 0 && frame_ids.count(frame_id2) > 0) {
        correspondence_graph_->AddCorrespondences(
            image_id1, image_id2, two_view_geometry.inlier_matches);
      } else {
        num_ignored_image_pairs += 1;
      }
    } else {
      num_ignored_image_pairs += 1;
    }
  }

  correspondence_graph_->Finalize();

  LOG(MM_INFO) << StringPrintf(" in %.3fs (ignored %d)",
                            timer.ElapsedSeconds(),
                            num_ignored_image_pairs);
}

std::shared_ptr<DatabaseCache> DatabaseCache::Create(
    const Database& database,
    const size_t min_num_matches,
    const bool ignore_watermarks,
    const std::unordered_set<std::string>& image_names) {
  auto cache = std::make_shared<DatabaseCache>();
  cache->Load(database, min_num_matches, ignore_watermarks, image_names);
  return cache;
}

void DatabaseCache::AddRig(class Rig rig) {
  const rig_t rig_id = rig.RigId();
  THROW_CHECK(!ExistsRig(rig_id));
  rigs_.emplace(rig_id, std::move(rig));
}

void DatabaseCache::AddCamera(struct Camera camera) {
  const camera_t camera_id = camera.camera_id;
  THROW_CHECK(!ExistsCamera(camera_id));
  cameras_.emplace(camera_id, std::move(camera));
}

void DatabaseCache::AddFrame(class Frame frame) {
  const rig_t frame_id = frame.FrameId();
  THROW_CHECK(!ExistsFrame(frame_id));
  frames_.emplace(frame_id, std::move(frame));
}

void DatabaseCache::AddImage(class Image image) {
  const image_t image_id = image.ImageId();
  THROW_CHECK(!ExistsImage(image_id));
  correspondence_graph_->AddImage(image_id, image.NumPoints2D());
  images_.emplace(image_id, std::move(image));
}

void DatabaseCache::AddPosePrior(image_t image_id,
                                 struct PosePrior pose_prior) {
  THROW_CHECK(ExistsImage(image_id));
  THROW_CHECK(!ExistsPosePrior(image_id));
  pose_priors_.emplace(image_id, std::move(pose_prior));
}

const class Image* DatabaseCache::FindImageWithName(
    const std::string& name) const {
  for (const auto& image : images_) {
    if (image.second.Name() == name) {
      return &image.second;
    }
  }
  return nullptr;
}

bool DatabaseCache::SetupPosePriors() {
  LOG(MM_INFO) << "Setting up prior positions...";

  Timer timer;
  timer.Start();

  if (NumPosePriors() == 0) {
    LOG(MM_ERROR) << "No pose priors in database...";
    return false;
  }

  bool prior_is_gps = true;

  // Get sorted image ids for GPS to cartesian conversion
  std::set<image_t> image_ids_with_prior;
  for (const auto& [image_id, _] : pose_priors_) {
    image_ids_with_prior.insert(image_id);
  }

  // Get GPS priors
  std::vector<Eigen::Vector3d> v_gps_prior;
  v_gps_prior.reserve(NumPosePriors());

  for (const image_t image_id : image_ids_with_prior) {
    const struct PosePrior& pose_prior = PosePrior(image_id);
    if (pose_prior.coordinate_system != PosePrior::CoordinateSystem::WGS84) {
      prior_is_gps = false;
    } else {
      // Image with the lowest id is to be used as the origin for prior
      // position conversion
      v_gps_prior.push_back(pose_prior.position);
    }
  }

  // Convert geographic to cartesian
  if (prior_is_gps) {
    // GPS reference to be used for EllipsoidToENU conversion
    const double ref_lat = v_gps_prior[0][0];
    const double ref_lon = v_gps_prior[0][1];

    const GPSTransform gps_transform(GPSTransform::Ellipsoid::WGS84);
    const std::vector<Eigen::Vector3d> v_xyz_prior =
        gps_transform.EllipsoidToENU(v_gps_prior, ref_lat, ref_lon);

    auto xyz_prior_it = v_xyz_prior.begin();
    for (const auto& image_id : image_ids_with_prior) {
      struct PosePrior& pose_prior = PosePrior(image_id);
      pose_prior.position = *xyz_prior_it;
      pose_prior.coordinate_system = PosePrior::CoordinateSystem::CARTESIAN;
      ++xyz_prior_it;
    }
  } else if (!prior_is_gps && !v_gps_prior.empty()) {
    LOG(MM_ERROR)
        << "Database is mixing GPS & non-GPS prior positions... Aborting";
    return false;
  }

  timer.PrintMinutes();

  return true;
}

}  // namespace colmap
