#include "Model.hpp"

#include "colmap/controllers/option_manager.h"
#include "colmap/estimators/alignment.h"
#include "colmap/estimators/coordinate_frame.h"
#include "colmap/geometry/gps.h"
#include "colmap/geometry/pose.h"
#include "colmap/optim/ransac.h"
#include "colmap/scene/reconstruction_io.h"
#include "colmap/sfm/observation_manager.h"
#include "colmap/util/file.h"
#include "colmap/util/misc.h"
#include "colmap/util/threading.h"

MM_NS_B

static std::vector<Eigen::AlignedBox3d> ComputeEqualPartsBboxes(
    const colmap::Reconstruction& reconstruction, const Eigen::Vector3i& split
) {
    std::vector<Eigen::AlignedBox3d> bboxes;
    const Eigen::AlignedBox3d bbox = reconstruction.ComputeBoundingBox();
    const Eigen::Vector3d extent = bbox.diagonal();
    const Eigen::Vector3d offset(
        extent(0) / split(0), extent(1) / split(1), extent(2) / split(2));

    for (int k = 0; k < split(2); ++k) {
        for (int j = 0; j < split(1); ++j) {
            for (int i = 0; i < split(0); ++i) {
                Eigen::Vector3d min(bbox.min().x() + i * offset(0),
                    bbox.min().z() + j * offset(1),
                    bbox.min().z() + k * offset(2));
                bboxes.emplace_back(min, min + offset);
            }
        }
    }

    return bboxes;
}

static Eigen::Vector3d TransformLatLonAltToModelCoords(const colmap::Sim3d& tform,
    const double lat,
    const double lon,
    const double alt
) {
    // Since this is intended for use in ENU aligned models we want to define the
    // altitude along the ENU frame z axis and not the Earth's radius. Thus, we
    // set the altitude to 0 when converting from LLA to ECEF and then we use the
    // altitude at the end, after scaling, to set it as the z coordinate in the
    // ENU frame.
    Eigen::Vector3d xyz =
        tform * colmap::GPSTransform(colmap::GPSTransform::Ellipsoid::WGS84)
        .EllipsoidToECEF({ Eigen::Vector3d(lat, lon, 0.0) })[0];
    xyz(2) = tform.scale * alt;
    return xyz;
}

static void WriteBoundingBox(const std::string& reconstruction_path,
    const Eigen::AlignedBox3d& bbox,
    const std::string& suffix = ""
) {
    const Eigen::Vector3d extent = bbox.diagonal();
    // write axis-aligned bounding box
    {
        const std::string path =
            colmap::JoinPaths(reconstruction_path, "bbox_aligned" + suffix + ".txt");
        std::ofstream file(path, std::ios::trunc);
        THROW_CHECK_FILE_OPEN(file, path);

        // Ensure that we don't lose any precision by storing in text.
        file.precision(17);
        file << bbox.min().transpose() << '\n';
        file << bbox.max().transpose() << '\n';
    }
    // write oriented bounding box
    {
        const std::string path =
            colmap::JoinPaths(reconstruction_path, "bbox_oriented" + suffix + ".txt");
        std::ofstream file(path, std::ios::trunc);
        THROW_CHECK_FILE_OPEN(file, path);

        // Ensure that we don't lose any precision by storing in text.
        file.precision(17);
        const Eigen::Vector3d center = (bbox.min() + bbox.max()) * 0.5;
        file << center.transpose() << "\n\n";
        file << "1 0 0\n0 1 0\n0 0 1\n\n";
        file << extent.transpose() << '\n';
    }
}

std::vector<Eigen::Vector3d> ConvertCameraLocations(
    const bool ref_is_gps,
    const std::string& alignment_type,
    const std::vector<Eigen::Vector3d>& ref_locations) {
    if (ref_is_gps) {
        const colmap::GPSTransform gps_transform(colmap::GPSTransform::Ellipsoid::WGS84);
        if (alignment_type != "enu") {
            LOG(INFO) << "Converting Alignment Coordinates from GPS (lat/lon/alt) "
                "to ECEF.";
            return gps_transform.EllipsoidToECEF(ref_locations);
        }
        else {
            LOG(INFO) << "Converting Alignment Coordinates from GPS (lat/lon/alt) "
                "to ENU.";
            return gps_transform.EllipsoidToENU(
                ref_locations, ref_locations[0](0), ref_locations[0](1));
        }
    }
    else {
        LOG(INFO) << "Cartesian Alignment Coordinates extracted (MUST NOT BE "
            "GPS coords!).";
        return ref_locations;
    }
}

static void ReadFileCameraLocations(const std::string& ref_images_path,
    const bool ref_is_gps,
    const std::string& alignment_type,
    std::vector<std::string>* ref_image_names,
    std::vector<Eigen::Vector3d>* ref_locations
) {
    for (const auto& line : colmap::ReadTextFileLines(ref_images_path)) {
        std::stringstream line_parser(line);
        std::string image_name;
        Eigen::Vector3d camera_position;
        line_parser >> image_name >> camera_position[0] >> camera_position[1] >>
            camera_position[2];
        ref_image_names->push_back(image_name);
        ref_locations->push_back(camera_position);
    }

    *ref_locations =
        ConvertCameraLocations(ref_is_gps, alignment_type, *ref_locations);
}

static void ReadDatabaseCameraLocations(const std::string& database_path,
    const bool ref_is_gps,
    const std::string& alignment_type,
    std::vector<std::string>* ref_image_names,
    std::vector<Eigen::Vector3d>* ref_locations
) {
    colmap::Database database(database_path);
    for (const auto& image : database.ReadAllImages()) {
        if (database.ExistsPosePrior(image.ImageId())) {
            ref_image_names->push_back(image.Name());
            const auto pose_prior = database.ReadPosePrior(image.ImageId());
            if (ref_is_gps) {
                THROW_CHECK_EQ(static_cast<int>(pose_prior.coordinate_system),
                    static_cast<int>(colmap::PosePrior::CoordinateSystem::WGS84));
            }
            ref_locations->push_back(pose_prior.position);
        }
    }

    *ref_locations =
        ConvertCameraLocations(ref_is_gps, alignment_type, *ref_locations);
}

static void WriteComparisonErrorsCSV(const std::string& path,
    const std::vector<colmap::ImageAlignmentError>& errors
) {
    std::ofstream file(path, std::ios::trunc);
    THROW_CHECK_FILE_OPEN(file, path);

    file.precision(17);
    file << "# Model comparison pose errors: one entry per common image\n";
    file << "# <rotation error (deg)>, <proj center error>\n";
    for (size_t i = 0; i < errors.size(); ++i) {
        file << errors[i].rotation_error_deg << ", " << errors[i].proj_center_error
            << '\n';
    }
}

static void PrintErrorStats(std::ostream& out, std::vector<double>& vals) {
    const size_t len = vals.size();
    if (len == 0) {
        out << "Cannot extract error statistics from empty input\n";
        return;
    }
    out << "Min:    " << colmap::Percentile(vals, 0) << '\n';
    out << "Max:    " << colmap::Percentile(vals, 100) << '\n';
    out << "Mean:   " << colmap::Mean(vals) << '\n';
    out << "Median: " << colmap::Median(vals) << '\n';
    out << "P90:    " << colmap::Percentile(vals, 90) << '\n';
    out << "P99:    " << colmap::Percentile(vals, 99) << '\n';
}

void PrintComparisonSummary(std::ostream& out,
    const std::vector<colmap::ImageAlignmentError>& errors
) {
    std::vector<double> rotation_errors_deg;
    rotation_errors_deg.reserve(errors.size());
    std::vector<double> proj_center_errors;
    proj_center_errors.reserve(errors.size());
    for (const auto& error : errors) {
        rotation_errors_deg.push_back(error.rotation_error_deg);
        proj_center_errors.push_back(error.proj_center_error);
    }
    out << "\nRotation errors (degrees)\n";
    PrintErrorStats(out, rotation_errors_deg);
    out << "\nProjection center errors\n";
    PrintErrorStats(out, proj_center_errors);
}

// Align given reconstruction with user provided cameras positions
// (can be used for geo-registration for instance).
// The cameras positions to be used for aligning the reconstruction
// model must be provided either by a txt file (with each line being: img_name x
// y z) or through a colmap database file containing a prior position for the
// registered images.
//
// Required Options:
// - input_path: path to initial reconstruction model
// - output_path: path to store the aligned reconstruction model
//
// Additional Options:
// - database_path: path to database file with prior positions for
// reconstruction images
// - ref_images_path: path to txt file with prior positions for reconstruction
// images (WARNING: provide only one of the above)
// - ref_is_gps: if true the prior positions are converted from GPS
// (lat/lon/alt) to ECEF or ENU
// - merge_image_and_ref_origins: if true the reconstruction will be shifted so
// that the first prior position is used for its camera position
// - transform_path: path to store the Sim3 transformation used for the
// alignment
// - alignment_type:
//    > plane: align with reconstruction principal plane
//    > ecef: align with ecef coords. (requires gps coords. or user provided
//    ecef coords.)
//    > enu: align with enu coords. (requires gps coords. or user provided enu
//    coords.)
//    > enu-plane: align to ecef and then to enu plane (requires gps
//    coords. or user provided ecef coords.)
//    > enu-plane-unscaled: same as above but do not apply the computed
//    scale when aligning the reconstruction
//    > custom: align to provided coords.
// - min_common_images: minimum number of images with prior positions to perform
// the estimate an alignment
// - estimate_scale: if true apply the computed scale when aligning the
// reconstruction
// - alignment_max_error: ransac error to use
int RunModelAligner(int argc, char** argv) {
    std::string input_path;
    std::string output_path;
    std::string database_path;
    std::string ref_model_path;
    std::string ref_images_path;
    bool ref_is_gps = true;
    bool merge_origins = false;
    std::string transform_path;
    std::string alignment_type = "custom";
    int min_common_images = 3;
    colmap::RANSACOptions ransac_options;

    colmap::OptionManager options;
    options.AddRequiredOption("input_path", &input_path);
    options.AddRequiredOption("output_path", &output_path);
    options.AddDefaultOption("database_path", &database_path);
    options.AddDefaultOption("ref_model_path", &ref_model_path);
    options.AddDefaultOption("ref_images_path", &ref_images_path);
    options.AddDefaultOption("ref_is_gps", &ref_is_gps);
    options.AddDefaultOption("merge_image_and_ref_origins", &merge_origins);
    options.AddDefaultOption("transform_path", &transform_path);
    options.AddDefaultOption(
        "alignment_type",
        &alignment_type,
        "{plane, ecef, enu, enu-plane, enu-plane-unscaled, custom}");
    options.AddDefaultOption("min_common_images", &min_common_images);
    options.AddDefaultOption("alignment_max_error", &ransac_options.max_error);
    options.Parse(argc, argv);

    colmap::StringToLower(&alignment_type);
    const std::unordered_set<std::string> alignment_options{
        "plane", "ecef", "enu", "enu-plane", "enu-plane-unscaled", "custom" };
    if (alignment_options.count(alignment_type) == 0) {
        LOG(ERROR) << "Invalid `alignment_type` - supported values are "
            "{'plane', 'ecef', 'enu', 'enu-plane', 'enu-plane-unscaled', "
            "'custom'}";
        return EXIT_FAILURE;
    }

    if (ransac_options.max_error <= 0) {
        LOG(ERROR) << "You must provide a maximum alignment error > 0";
        return EXIT_FAILURE;
    }

    if (ref_model_path.empty() && database_path.empty() &&
        ref_images_path.empty() && alignment_type != "plane") {
        LOG(ERROR) << "One of the following arguments must be specified: "
            "--ref_model_path, --database_path, "
            "--ref_images_path, --alignment_type=plane";
        return EXIT_FAILURE;
    }

    std::vector<std::string> ref_image_names;
    std::vector<Eigen::Vector3d> ref_locations;
    if (!ref_model_path.empty() && database_path.empty()) {
        colmap::Reconstruction reconstruction;
        reconstruction.Read(ref_model_path);
        for (const auto& image : reconstruction.Images()) {
            ref_image_names.push_back(image.second.Name());
            ref_locations.push_back(image.second.ProjectionCenter());
        }
    }
    else if (!ref_images_path.empty() && database_path.empty()) {
        ReadFileCameraLocations(ref_images_path,
            ref_is_gps,
            alignment_type,
            &ref_image_names,
            &ref_locations);
    }
    else if (!database_path.empty() && ref_images_path.empty()) {
        ReadDatabaseCameraLocations(database_path,
            ref_is_gps,
            alignment_type,
            &ref_image_names,
            &ref_locations);
    }
    else if (alignment_type != "plane") {
        LOG(ERROR) << "Use location file or database, not both";
        return EXIT_FAILURE;
    }

    if (alignment_type != "plane" &&
        static_cast<int>(ref_locations.size()) < min_common_images) {
        LOG(ERROR) << "Cannot align with insufficient reference locations.";
        return EXIT_FAILURE;
    }

    colmap::Reconstruction reconstruction;
    reconstruction.Read(input_path);
    colmap::Sim3d tform;

    if (alignment_type == "plane") {
        colmap::PrintHeading2("Aligning reconstruction to principal plane");
        AlignToPrincipalPlane(&reconstruction, &tform);
    }
    else {
        colmap::PrintHeading2("Aligning reconstruction to " + alignment_type);
        LOG(INFO) << colmap::StringPrintf("=> Using %d reference images",
            ref_image_names.size());

        const bool alignment_success =
            colmap::AlignReconstructionToLocations(reconstruction,
                ref_image_names,
                ref_locations,
                min_common_images,
                ransac_options,
                &tform);

        if (!alignment_success) {
            LOG(ERROR) << "=> Alignment failed";
            return EXIT_FAILURE;
        }

        reconstruction.Transform(tform);

        std::vector<double> errors;
        errors.reserve(ref_image_names.size());

        for (size_t i = 0; i < ref_image_names.size(); ++i) {
            const colmap::Image* image = reconstruction.FindImageWithName(ref_image_names[i]);
            if (image != nullptr) {
                errors.push_back((image->ProjectionCenter() - ref_locations[i]).norm());
            }
        }
        LOG(INFO) << colmap::StringPrintf("=> Alignment error: %f (mean), %f (median)",
            colmap::Mean(errors),
            colmap::Median(errors));

        if (alignment_success && colmap::StringStartsWith(alignment_type, "enu-plane")) {
            colmap::PrintHeading2("Aligning ECEF aligned reconstruction to ENU plane");
            colmap::AlignToENUPlane(
                &reconstruction, &tform, alignment_type == "enu-plane-unscaled");
        }
    }

    if (merge_origins) {
        for (size_t i = 0; i < ref_image_names.size(); i++) {
            const colmap::Image* first_image =
                reconstruction.FindImageWithName(ref_image_names[i]);

            if (first_image != nullptr) {
                const Eigen::Vector3d& first_img_position = ref_locations[i];
                const Eigen::Vector3d trans_align =
                    first_img_position - first_image->ProjectionCenter();
                const colmap::Sim3d origin_align(
                    1.0, Eigen::Quaterniond::Identity(), trans_align);

                LOG(INFO) << "\n Aligning reconstruction's origin with ref origin: "
                    << first_img_position.transpose() << '\n';

                reconstruction.Transform(origin_align);

                // Update the Sim3 transformation in case it is stored next.
                tform = colmap::Sim3d(tform.scale, tform.rotation, tform.translation + trans_align);

                break;
            }
        }
    }

    LOG(INFO) << "=> Alignment succeeded";
    reconstruction.Write(output_path);
    if (!transform_path.empty()) {
        tform.ToFile(transform_path);
    }

    return EXIT_SUCCESS;
}

int RunModelAnalyzer(int argc, char** argv) {
    std::string path;
    bool verbose = false;

    colmap::OptionManager options;
    options.AddRequiredOption("path", &path);
    options.AddDefaultOption("verbose", &verbose);
    options.Parse(argc, argv);

    colmap::Reconstruction reconstruction;
    reconstruction.Read(path);

    LOG(INFO) << colmap::StringPrintf("Rigs: %d", reconstruction.NumRigs());
    LOG(INFO) << colmap::StringPrintf("Cameras: %d", reconstruction.NumCameras());
    LOG(INFO) << colmap::StringPrintf("Frames: %d", reconstruction.NumFrames());
    LOG(INFO) << colmap::StringPrintf("Registered frames: %d",
        reconstruction.NumRegFrames());
    LOG(INFO) << colmap::StringPrintf("Images: %d", reconstruction.NumImages());
    LOG(INFO) << colmap::StringPrintf("Registered images: %d",
        reconstruction.NumRegImages());
    LOG(INFO) << colmap::StringPrintf("Points: %d", reconstruction.NumPoints3D());
    LOG(INFO) << colmap::StringPrintf("Observations: %d",
        reconstruction.ComputeNumObservations());
    LOG(INFO) << colmap::StringPrintf("Mean track length: %f",
        reconstruction.ComputeMeanTrackLength());
    LOG(INFO) << colmap::StringPrintf(
        "Mean observations per image: %f",
        reconstruction.ComputeMeanObservationsPerRegImage());
    LOG(INFO) << colmap::StringPrintf("Mean reprojection error: %fpx",
        reconstruction.ComputeMeanReprojectionError());

    // verbose information
    if (verbose) {
        colmap::PrintHeading2("Cameras");
        for (const auto& camera : reconstruction.Cameras()) {
            LOG(INFO) << colmap::StringPrintf(" - Camera Id: %d, Model Name: %s, Params: %s",
                camera.first,
                camera.second.ModelName().c_str(),
                camera.second.ParamsToString().c_str());
        }

        colmap::PrintHeading2("Images");
        for (const auto& image_id : reconstruction.RegImageIds()) {
            LOG(INFO) << colmap::StringPrintf(" - Registered Image Id: %d, Name: %s",
                image_id,
                reconstruction.Image(image_id).Name().c_str());
        }
    }

    return EXIT_SUCCESS;
}

int RunModelComparer(int argc, char** argv) {
    std::string input_path1;
    std::string input_path2;
    std::string output_path;
    std::string alignment_error = "reprojection";
    double min_inlier_observations = 0.3;
    double max_reproj_error = 8.0;
    double max_proj_center_error = 0.1;

    colmap::OptionManager options;
    options.AddRequiredOption("input_path1", &input_path1);
    options.AddRequiredOption("input_path2", &input_path2);
    options.AddDefaultOption("output_path", &output_path);
    options.AddDefaultOption(
        "alignment_error", &alignment_error, "{reprojection, proj_center}");
    options.AddDefaultOption("min_inlier_observations", &min_inlier_observations);
    options.AddDefaultOption("max_reproj_error", &max_reproj_error);
    options.AddDefaultOption("max_proj_center_error", &max_proj_center_error);
    options.Parse(argc, argv);

    if (!output_path.empty() && !colmap::ExistsDir(output_path)) {
        LOG(ERROR) << "Provided output path is not a valid directory";
        return EXIT_FAILURE;
    }

    colmap::Reconstruction reconstruction1;
    reconstruction1.Read(input_path1);
    colmap::Reconstruction reconstruction2;
    reconstruction2.Read(input_path2);
    std::vector<colmap::ImageAlignmentError> errors;
    colmap::Sim3d rec2_from_rec1;
    bool success = CompareModels(reconstruction1,
        reconstruction2,
        alignment_error,
        min_inlier_observations,
        max_reproj_error,
        max_proj_center_error,
        errors,
        rec2_from_rec1);
    if (!success) {
        return EXIT_FAILURE;
    }
    if (!output_path.empty()) {
        const std::string errors_path = colmap::JoinPaths(output_path, "errors.csv");
        WriteComparisonErrorsCSV(errors_path, errors);
        const std::string summary_path =
            colmap::JoinPaths(output_path, "errors_summary.txt");
        std::ofstream file(summary_path, std::ios::trunc);
        THROW_CHECK_FILE_OPEN(file, summary_path);
        PrintComparisonSummary(file, errors);
    }
    return EXIT_SUCCESS;
}

bool CompareModels(const colmap::Reconstruction& reconstruction1,
    const colmap::Reconstruction& reconstruction2,
    const std::string& alignment_error,
    const double min_inlier_observations,
    const double max_reproj_error,
    const double max_proj_center_error,
    std::vector<colmap::ImageAlignmentError>& errors,
    colmap::Sim3d& rec2_from_rec1
) {
    colmap::PrintHeading1("Reconstruction 1");
    LOG(INFO) << colmap::StringPrintf("Frames: %d", reconstruction1.NumRegFrames());
    LOG(INFO) << colmap::StringPrintf("Images: %d", reconstruction1.NumRegImages());
    LOG(INFO) << colmap::StringPrintf("Points: %d", reconstruction1.NumPoints3D());

    colmap::PrintHeading1("Reconstruction 2");
    LOG(INFO) << colmap::StringPrintf("Frames: %d", reconstruction2.NumRegFrames());
    LOG(INFO) << colmap::StringPrintf("Images: %d", reconstruction2.NumRegImages());
    LOG(INFO) << colmap::StringPrintf("Points: %d", reconstruction2.NumPoints3D());

    colmap::PrintHeading1("Comparing reconstructed image poses");
    const std::vector<std::pair<colmap::image_t, colmap::image_t>> common_image_ids =
        reconstruction1.FindCommonRegImageIds(reconstruction2);
    LOG(INFO) << colmap::StringPrintf("Common images: %d", common_image_ids.size());

    bool success = false;
    if (alignment_error == "reprojection") {
        success = colmap::AlignReconstructionsViaReprojections(
            reconstruction1,
            reconstruction2,
            /*min_inlier_observations=*/min_inlier_observations,
            /*max_reproj_error=*/max_reproj_error,
            &rec2_from_rec1);
    }
    else if (alignment_error == "proj_center") {
        success = colmap::AlignReconstructionsViaProjCenters(
            reconstruction1,
            reconstruction2,
            /*max_proj_center_error=*/max_proj_center_error,
            &rec2_from_rec1);
    }
    else {
        LOG(ERROR) << "Invalid alignment_error specified.";
        return false;
    }

    if (!success) {
        LOG(INFO) << "=> Reconstruction alignment failed";
        return false;
    }

    LOG(INFO) << "Computed alignment transform:\n" << rec2_from_rec1.ToMatrix();

    errors = ComputeImageAlignmentError(
        reconstruction1, reconstruction2, rec2_from_rec1);

    colmap::PrintHeading2("Image alignment error summary");
    PrintComparisonSummary(std::cout, errors);

    return true;
}

int RunModelConverter(int argc, char** argv) {
    std::string input_path;
    std::string output_path;
    std::string output_type;
    bool skip_distortion = false;

    colmap::OptionManager options;
    options.AddRequiredOption("input_path", &input_path);
    options.AddRequiredOption("output_path", &output_path);
    options.AddRequiredOption("output_type",
        &output_type,
        "{BIN, TXT, NVM, Bundler, VRML, PLY, R3D, CAM}");
    options.AddDefaultOption("skip_distortion", &skip_distortion);
    options.Parse(argc, argv);

    colmap::Reconstruction reconstruction;
    reconstruction.Read(input_path);

    colmap::StringToLower(&output_type);
    if (output_type == "bin") {
        reconstruction.WriteBinary(output_path);
    }
    else if (output_type == "txt") {
        reconstruction.WriteText(output_path);
    }
    else if (output_type == "nvm") {
        colmap::ExportNVM(reconstruction, output_path, skip_distortion);
    }
    else if (output_type == "bundler") {
        colmap::ExportBundler(reconstruction,
            output_path + ".bundle.out",
            output_path + ".list.txt",
            skip_distortion);
    }
    else if (output_type == "r3d") {
        colmap::ExportRecon3D(reconstruction, output_path, skip_distortion);
    }
    else if (output_type == "cam") {
        colmap::ExportCam(reconstruction, output_path, skip_distortion);
    }
    else if (output_type == "ply") {
        colmap::ExportPLY(reconstruction, output_path);
    }
    else if (output_type == "vrml") {
        const auto base_path = output_path.substr(0, output_path.find_last_of('.'));
        colmap::ExportVRML(reconstruction,
            base_path + ".images.wrl",
            base_path + ".points3D.wrl",
            1,
            Eigen::Vector3d(1, 0, 0));
    }
    else {
        LOG(ERROR) << "Invalid `output_type`";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int RunModelCropper(int argc, char** argv) {
    colmap::Timer timer;
    timer.Start();

    std::string input_path;
    std::string output_path;
    std::string boundary;
    std::string gps_transform_path;
    bool is_gps = false;

    colmap::OptionManager options;
    options.AddRequiredOption("input_path", &input_path);
    options.AddRequiredOption("output_path", &output_path);
    options.AddRequiredOption("boundary", &boundary);
    options.AddDefaultOption("gps_transform_path", &gps_transform_path);
    options.Parse(argc, argv);

    if (!colmap::ExistsDir(input_path)) {
        LOG(ERROR) << "`input_path` is not a directory";
        return EXIT_FAILURE;
    }

    if (!colmap::ExistsDir(output_path)) {
        LOG(ERROR) << "`output_path` is not a directory";
        return EXIT_FAILURE;
    }

    std::vector<double> boundary_elements = colmap::CSVToVector<double>(boundary);
    if (boundary_elements.size() != 2 && boundary_elements.size() != 6) {
        LOG(ERROR) << "Invalid `boundary` - supported values are "
            "'x1,y1,z1,x2,y2,z2' or 'p1,p2'.";
        return EXIT_FAILURE;
    }

    colmap::Reconstruction reconstruction;
    reconstruction.Read(input_path);

    colmap::PrintHeading2("Calculating boundary coordinates");
    Eigen::AlignedBox3d bounding_box;
    if (boundary_elements.size() == 6) {
        colmap::Sim3d tform;
        if (!gps_transform_path.empty()) {
            colmap::PrintHeading2("Reading model to ECEF transform");
            is_gps = true;
            tform = colmap::Inverse(colmap::Sim3d::FromFile(gps_transform_path));
        }
        bounding_box.min() =
            is_gps ? TransformLatLonAltToModelCoords(tform,
                boundary_elements[0],
                boundary_elements[1],
                boundary_elements[2])
            : Eigen::Vector3d(boundary_elements[0],
                boundary_elements[1],
                boundary_elements[2]);
        bounding_box.max() =
            is_gps ? TransformLatLonAltToModelCoords(tform,
                boundary_elements[3],
                boundary_elements[4],
                boundary_elements[5])
            : Eigen::Vector3d(boundary_elements[3],
                boundary_elements[4],
                boundary_elements[5]);
    }
    else {
        bounding_box = reconstruction.ComputeBoundingBox(boundary_elements[0],
            boundary_elements[1]);
    }

    colmap::PrintHeading2("Cropping reconstruction");
    reconstruction.Crop(bounding_box).Write(output_path);
    WriteBoundingBox(output_path, bounding_box);

    LOG(INFO) << "=> Cropping succeeded";
    timer.PrintMinutes();
    return EXIT_SUCCESS;
}

int RunModelMerger(int argc, char** argv) {
    std::string input_path1;
    std::string input_path2;
    std::string output_path;
    double max_reproj_error = 64.0;

    colmap::OptionManager options;
    options.AddRequiredOption("input_path1", &input_path1);
    options.AddRequiredOption("input_path2", &input_path2);
    options.AddRequiredOption("output_path", &output_path);
    options.AddDefaultOption("max_reproj_error", &max_reproj_error);
    options.Parse(argc, argv);

    colmap::Reconstruction reconstruction1;
    reconstruction1.Read(input_path1);
    colmap::PrintHeading2("Reconstruction 1");
    LOG(INFO) << colmap::StringPrintf("Images: %d", reconstruction1.NumRegImages());
    LOG(INFO) << colmap::StringPrintf("Points: %d", reconstruction1.NumPoints3D());

    colmap::Reconstruction reconstruction2;
    reconstruction2.Read(input_path2);
    colmap::PrintHeading2("Reconstruction 2");
    LOG(INFO) << colmap::StringPrintf("Images: %d", reconstruction2.NumRegImages());
    LOG(INFO) << colmap::StringPrintf("Points: %d", reconstruction2.NumPoints3D());

    colmap::PrintHeading2("Merging reconstructions");
    if (colmap::MergeAndFilterReconstructions(
        max_reproj_error, reconstruction1, reconstruction2)) {
        LOG(INFO) << "=> Merge succeeded";
        colmap::PrintHeading2("Merged reconstruction");
        LOG(INFO) << colmap::StringPrintf("Images: %d", reconstruction2.NumRegImages());
        LOG(INFO) << colmap::StringPrintf("Points: %d", reconstruction2.NumPoints3D());
    }
    else {
        LOG(INFO) << "=> Merge failed";
    }

    reconstruction2.Write(output_path);

    return EXIT_SUCCESS;
}

int RunModelOrientationAligner(int argc, char** argv) {
    std::string input_path;
    std::string output_path;
#ifdef COLMAP_LSD_ENABLED
    std::string method = "MANHATTAN-WORLD";
#else
    std::string method = "IMAGE-ORIENTATION";
#endif

    colmap::ManhattanWorldFrameEstimationOptions frame_estimation_options;

    colmap::OptionManager options;
    options.AddImageOptions();
    options.AddRequiredOption("input_path", &input_path);
    options.AddRequiredOption("output_path", &output_path);
#ifdef COLMAP_LSD_ENABLED
    options.AddDefaultOption(
        "method", &method, "{MANHATTAN-WORLD, IMAGE-ORIENTATION}");
#else
    options.AddDefaultOption("method", &method, "{IMAGE-ORIENTATION}");
#endif
    options.AddDefaultOption("max_image_size",
        &frame_estimation_options.max_image_size);
    options.Parse(argc, argv);

    colmap::StringToLower(&method);
#ifdef COLMAP_LSD_ENABLED
    if (method != "manhattan-world" && method != "image-orientation") {
        LOG(ERROR) << "Invalid `method` - supported values are "
            "'MANHATTAN-WORLD' or 'IMAGE-ORIENTATION'.";
        return EXIT_FAILURE;
    }
#else
    if (method != "image-orientation") {
        LOG(ERROR) << "Invalid `method` - supported values are "
            "'IMAGE-ORIENTATION'.";
        return EXIT_FAILURE;
    }
#endif

    colmap::Reconstruction reconstruction;
    reconstruction.Read(input_path);

    colmap::PrintHeading1("Aligning Reconstruction");

    colmap::Sim3d new_from_old_world;

#ifdef COLMAP_LSD_ENABLED
    if (method == "manhattan-world") {
        const Eigen::Matrix3d frame = EstimateManhattanWorldFrame(
            frame_estimation_options, reconstruction, *options.image_path);

        if (frame.col(0).lpNorm<1>() == 0) {
            LOG(INFO) << "Only aligning vertical axis";
            new_from_old_world.rotation = Eigen::Quaterniond::FromTwoVectors(
                frame.col(1), Eigen::Vector3d(0, 1, 0));
        }
        else if (frame.col(1).lpNorm<1>() == 0) {
            new_from_old_world.rotation = Eigen::Quaterniond::FromTwoVectors(
                frame.col(0), Eigen::Vector3d(1, 0, 0));
            LOG(INFO) << "Only aligning horizontal axis";
        }
        else {
            new_from_old_world.rotation = Eigen::Quaterniond(frame.transpose());
            LOG(INFO) << "Aligning horizontal and vertical axes";
        }
    }
    else if (method == "image-orientation") {
#else
    if (method == "image-orientation") {
#endif
        const Eigen::Vector3d gravity_axis =
            colmap::EstimateGravityVectorFromImageOrientation(reconstruction);
        new_from_old_world.rotation = Eigen::Quaterniond::FromTwoVectors(
            gravity_axis, Eigen::Vector3d(0, 1, 0));

    }
    else {
        LOG(FATAL_THROW) << "Alignment method not supported";
    }

    LOG(INFO) << "Using the rotation matrix:";
    LOG(INFO) << new_from_old_world.rotation.toRotationMatrix();

    reconstruction.Transform(new_from_old_world);

    LOG(INFO) << "Writing aligned reconstruction...";
    reconstruction.Write(output_path);

    return EXIT_SUCCESS;
    }

int RunModelSplitter(int argc, char** argv) {
    colmap::Timer timer;
    timer.Start();

    std::string input_path;
    std::string output_path;
    std::string split_type;
    std::string split_params;
    std::string gps_transform_path;
    int num_threads = -1;
    int min_reg_images = 10;
    int min_num_points = 100;
    double overlap_ratio = 0.0;
    double min_area_ratio = 0.0;
    bool is_gps = false;

    colmap::OptionManager options;
    options.AddRequiredOption("input_path", &input_path);
    options.AddRequiredOption("output_path", &output_path);
    options.AddRequiredOption(
        "split_type", &split_type, "{tiles, extent, parts}");
    options.AddRequiredOption("split_params", &split_params);
    options.AddDefaultOption("gps_transform_path", &gps_transform_path);
    options.AddDefaultOption("num_threads", &num_threads);
    options.AddDefaultOption("min_reg_images", &min_reg_images);
    options.AddDefaultOption("min_num_points", &min_num_points);
    options.AddDefaultOption("overlap_ratio", &overlap_ratio);
    options.AddDefaultOption("min_area_ratio", &min_area_ratio);
    options.Parse(argc, argv);

    if (!colmap::ExistsDir(input_path)) {
        LOG(ERROR) << "`input_path` is not a directory";
        return EXIT_FAILURE;
    }

    if (!colmap::ExistsDir(output_path)) {
        LOG(ERROR) << "`output_path` is not a directory";
        return EXIT_FAILURE;
    }

    if (overlap_ratio < 0) {
        LOG(WARNING) << "Invalid `overlap_ratio`; resetting to 0";
        overlap_ratio = 0.0;
    }

    colmap::PrintHeading1("Splitting sparse model");
    LOG(INFO) << colmap::StringPrintf("=> Using \"%s\" split type", split_type.c_str());

    colmap::Reconstruction reconstruction;
    reconstruction.Read(input_path);

    colmap::Sim3d tform;
    if (!gps_transform_path.empty()) {
        colmap::PrintHeading2("Reading model to ECEF transform");
        is_gps = true;
        tform = colmap::Inverse(colmap::Sim3d::FromFile(gps_transform_path));
    }

    // Create the necessary number of reconstructions based on the split method
    // and get the bounding boxes for each sub-reconstruction
    colmap::PrintHeading2("Computing bounding boxes");
    std::vector<std::string> tile_keys;
    std::vector<Eigen::AlignedBox3d> exact_bboxes;
    colmap::StringToLower(&split_type);
    if (split_type == "tiles") {
        std::ifstream file(split_params);
        THROW_CHECK_FILE_OPEN(file, split_params);

        double x1, y1, z1, x2, y2, z2;
        std::string tile_key;
        std::vector<Eigen::AlignedBox3d> bounds;
        tile_keys.clear();
        file >> tile_key >> x1 >> y1 >> z1 >> x2 >> y2 >> z2;
        while (!file.fail()) {
            tile_keys.push_back(tile_key);
            if (is_gps) {
                exact_bboxes.emplace_back(
                    TransformLatLonAltToModelCoords(tform, x1, y1, z1),
                    TransformLatLonAltToModelCoords(tform, x2, y2, z2));
            }
            else {
                exact_bboxes.emplace_back(Eigen::Vector3d(x1, y1, z1),
                    Eigen::Vector3d(x2, y2, z2));
            }
            file >> tile_key >> x1 >> y1 >> z1 >> x2 >> y2 >> z2;
        }
    }
    else if (split_type == "extent") {
        std::vector<double> parts = colmap::CSVToVector<double>(split_params);
        Eigen::Vector3d extent(std::numeric_limits<double>::max(),
            std::numeric_limits<double>::max(),
            std::numeric_limits<double>::max());
        for (size_t i = 0; i < parts.size(); ++i) {
            extent(i) = parts[i] * tform.scale;
        }

        const Eigen::AlignedBox3d bbox = reconstruction.ComputeBoundingBox();
        const Eigen::Vector3d full_bbox = bbox.diagonal();
        const Eigen::Vector3i split(static_cast<int>(full_bbox(0) / extent(0)) + 1,
            static_cast<int>(full_bbox(1) / extent(1)) + 1,
            static_cast<int>(full_bbox(2) / extent(2)) + 1);

        exact_bboxes = ComputeEqualPartsBboxes(reconstruction, split);
    }
    else if (split_type == "parts") {
        auto parts = colmap::CSVToVector<int>(split_params);
        Eigen::Vector3i split(1, 1, 1);
        for (size_t i = 0; i < parts.size(); ++i) {
            split(i) = parts[i];
            if (split(i) < 1) {
                LOG(ERROR) << "Cannot split in less than 1 parts for dim " << i;
                return EXIT_FAILURE;
            }
        }
        exact_bboxes = ComputeEqualPartsBboxes(reconstruction, split);
    }
    else {
        LOG(ERROR) << "Invalid split type: " << split_type;
        return EXIT_FAILURE;
    }

    std::vector<Eigen::AlignedBox3d> padded_bboxes;
    for (const auto& bbox : exact_bboxes) {
        const Eigen::Vector3d padding = overlap_ratio * bbox.diagonal();
        padded_bboxes.emplace_back(bbox.min() - padding, bbox.max() + padding);
    }

    colmap::PrintHeading2("Applying split and writing reconstructions");
    const size_t num_parts = padded_bboxes.size();
    LOG(INFO) << colmap::StringPrintf("=> Splitting to %d parts", num_parts);

    const bool use_tile_keys = split_type == "tiles";

    auto SplitReconstruction = [&](const int idx) {
        colmap::Reconstruction tile_recon = reconstruction.Crop(padded_bboxes[idx]);
        // calculate area covered by model as proportion of box area
        const Eigen::Vector3d bbox_extent = padded_bboxes[idx].diagonal();
        const Eigen::AlignedBox3d model_bbox = tile_recon.ComputeBoundingBox();
        const Eigen::Vector3d model_extent = model_bbox.diagonal();
        const double area_ratio =
            (model_extent(0) * model_extent(1)) / (bbox_extent(0) * bbox_extent(1));
        const int tile_num_points = tile_recon.NumPoints3D();

        const std::string name =
            use_tile_keys ? tile_keys[idx] : std::to_string(idx);
        const bool include_tile =
            area_ratio >= min_area_ratio &&       //
            tile_num_points >= min_num_points &&  //
            tile_recon.NumRegImages() >= static_cast<size_t>(min_reg_images);

        if (include_tile) {
            LOG(INFO) << colmap::StringPrintf(
                "Writing reconstruction %s with %d images, %d points, "
                "and %.2f%% area coverage",
                name.c_str(),
                tile_recon.NumRegImages(),
                tile_num_points,
                100.0 * area_ratio);
            const std::string reconstruction_path = colmap::JoinPaths(output_path, name);
            colmap::CreateDirIfNotExists(reconstruction_path);
            tile_recon.Write(reconstruction_path);
            WriteBoundingBox(reconstruction_path, padded_bboxes[idx]);
            WriteBoundingBox(reconstruction_path, exact_bboxes[idx], "_exact");
        }
        else {
            LOG(INFO) << colmap::StringPrintf(
                "Skipping reconstruction %s with %d images, %d points, "
                "and %.2f%% area coverage",
                name.c_str(),
                tile_recon.NumRegImages(),
                tile_num_points,
                100.0 * area_ratio);
        }
        };

    colmap::ThreadPool thread_pool(colmap::GetEffectiveNumThreads(num_threads));
    for (size_t idx = 0; idx < num_parts; ++idx) {
        thread_pool.AddTask(SplitReconstruction, idx);
    }
    thread_pool.Wait();

    timer.PrintMinutes();
    return EXIT_SUCCESS;
}

int RunModelTransformer(int argc, char** argv) {
    std::string input_path;
    std::string output_path;
    std::string transform_path;
    bool is_inverse = false;

    colmap::OptionManager options;
    options.AddRequiredOption("input_path", &input_path);
    options.AddRequiredOption("output_path", &output_path);
    options.AddRequiredOption("transform_path", &transform_path);
    options.AddDefaultOption("is_inverse", &is_inverse);
    options.Parse(argc, argv);

    LOG(INFO) << "Reading points input: " << input_path;
    colmap::Reconstruction recon;
    bool is_dense = false;
    if (colmap::HasFileExtension(input_path, ".ply")) {
        is_dense = true;
        recon.ImportPLY(input_path);
    }
    else if (colmap::ExistsDir(input_path)) {
        recon.Read(input_path);
    }
    else {
        LOG(ERROR)
            << "Invalid model input; not a PLY file or sparse reconstruction "
            "directory.";
        return EXIT_FAILURE;
    }

    LOG(INFO) << "Reading transform input: " << transform_path;
    colmap::Sim3d tform = colmap::Sim3d::FromFile(transform_path);
    if (is_inverse) {
        tform = colmap::Inverse(tform);
    }

    LOG(INFO) << "Applying transform to recon with " << recon.NumPoints3D()
        << " points";
    recon.Transform(tform);

    LOG(INFO) << "Writing output: " << output_path;
    if (is_dense) {
        colmap::ExportPLY(recon, output_path);
    }
    else {
        recon.Write(output_path);
    }

    return EXIT_SUCCESS;
}

MM_NS_E