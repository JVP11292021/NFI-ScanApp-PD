#include "MapModel.hpp"

#include <controllers/option_manager.h>
#include <estimators/alignment.h>
#include <scene/reconstruction_io.h>
#include <util/file.h>

MM_NS_B

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


int RunModelConverter(
    const std::filesystem::path& input_path,
    const std::filesystem::path& output_path,
    const std::string& output_type,
    bool skip_distortion
) {
    colmap::Reconstruction reconstruction;
    reconstruction.Read(input_path.string());

    std::string type = output_type;
    colmap::StringToLower(&type);

    if (type == "bin") {
        reconstruction.WriteBinary(output_path.string());
    }
    else if (type == "txt") {
        reconstruction.WriteText(output_path.string());
    }
    else if (type == "nvm") {
        colmap::ExportNVM(reconstruction, output_path.string(), skip_distortion);
    }
    else if (type == "bundler") {
        colmap::ExportBundler(
            reconstruction,
            output_path.string() + ".bundle.out",
            output_path.string() + ".list.txt",
            skip_distortion
        );
    }
    else if (type == "r3d") {
        colmap::ExportRecon3D(reconstruction, output_path.string(), skip_distortion);
    }
    else if (type == "cam") {
        colmap::ExportCam(reconstruction, output_path.string(), skip_distortion);
    }
    else if (type == "ply") {
        colmap::ExportPLY(reconstruction, output_path.string());
    }
    else if (type == "vrml") {
        const auto base_path = output_path.string().substr(0, output_path.string().find_last_of('.'));
        colmap::ExportVRML(
            reconstruction,
            base_path + ".images.wrl",
            base_path + ".points3D.wrl",
            1,
            Eigen::Vector3d(1, 0, 0)
        );
    }
    else {
        LOG(MM_ERROR) << "Invalid `output_type`";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

MM_NS_E