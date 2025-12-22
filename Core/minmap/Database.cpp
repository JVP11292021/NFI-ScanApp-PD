#include "Database.hpp"

#include "colmap/controllers/option_manager.h"
#include "colmap/geometry/pose.h"
#include "colmap/scene/database.h"
#include "colmap/scene/reconstruction.h"
#include "colmap/scene/rig.h"
#include "colmap/util/file.h"
#include "colmap/util/misc.h"

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

MM_NS_B

int RunDatabaseCleaner(int argc, char** argv) {
    std::string type;

    colmap::OptionManager options;
    options.AddRequiredOption("type", &type, "{all, images, features, matches}");
    options.AddDatabaseOptions();
    options.Parse(argc, argv);

    colmap::StringToLower(&type);
    colmap::Database database(*options.database_path);
    colmap::PrintHeading1("Clearing database");
    {
        colmap::DatabaseTransaction transaction(&database);
        if (type == "all") {
            colmap::PrintHeading2("Clearing all tables");
            database.ClearAllTables();
        }
        else if (type == "images") {
            colmap::PrintHeading2("Clearing Images and all dependent tables");
            database.ClearImages();
            database.ClearTwoViewGeometries();
            database.ClearMatches();
        }
        else if (type == "features") {
            colmap::PrintHeading2("Clearing image features and matches");
            database.ClearDescriptors();
            database.ClearKeypoints();
            database.ClearTwoViewGeometries();
            database.ClearMatches();
        }
        else if (type == "matches") {
            colmap::PrintHeading2("Clearing image matches");
            database.ClearTwoViewGeometries();
            database.ClearMatches();
        }
        else {
            LOG(ERROR) << "Invalid cleanup type; no changes in database";
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

int RunDatabaseCreator(int argc, char** argv) {
    colmap::OptionManager options;
    options.AddDatabaseOptions();
    options.Parse(argc, argv);

    colmap::Database database(*options.database_path);

    return EXIT_SUCCESS;
}

int RunDatabaseMerger(int argc, char** argv) {
    std::string database_path1;
    std::string database_path2;
    std::string merged_database_path;

    colmap::OptionManager options;
    options.AddRequiredOption("database_path1", &database_path1);
    options.AddRequiredOption("database_path2", &database_path2);
    options.AddRequiredOption("merged_database_path", &merged_database_path);
    options.Parse(argc, argv);

    if (colmap::ExistsFile(merged_database_path)) {
        LOG(ERROR) << "Merged database file must not exist.";
        return EXIT_FAILURE;
    }

    colmap::Database database1(database_path1);
    colmap::Database database2(database_path2);
    colmap::Database merged_database(merged_database_path);
    colmap::Database::Merge(database1, database2, &merged_database);

    return EXIT_SUCCESS;
}

int RunRigConfigurator(int argc, char** argv) {
    std::string database_path;
    std::string rig_config_path;
    std::string input_path;
    std::string output_path;

    colmap::OptionManager options;
    options.AddRequiredOption("database_path", &database_path);
    options.AddRequiredOption("rig_config_path",
        &rig_config_path,
        "Rig configuration as a .json file.");
    options.AddDefaultOption("input_path",
        &input_path,
        "Optional input reconstruction to automatically "
        "derive the (average) rig and camera calibrations. "
        "If not provided, the rig intrinsics and extrinsics "
        "must be specified in the provided config.");
    options.AddDefaultOption(
        "output_path",
        &output_path,
        "Optional output reconstruction with configured rigs/frames.");
    options.Parse(argc, argv);

    std::optional<colmap::Reconstruction> reconstruction;
    if (!input_path.empty()) {
        reconstruction = std::make_optional<colmap::Reconstruction>();
        reconstruction->Read(input_path);
    }

    colmap::Database database(database_path);

    colmap::ApplyRigConfig(
        colmap::ReadRigConfig(rig_config_path),
        database,
        reconstruction.has_value() ? &reconstruction.value() : nullptr);

    if (reconstruction.has_value() && !output_path.empty()) {
        reconstruction->Write(output_path);
    }

    return EXIT_SUCCESS;
}

MM_NS_E