//#include <minmap.hpp>
//
//typedef std::function<int(int, char**)> command_func_t;
//
//int ShowHelp(
//    const std::vector<std::pair<std::string, command_func_t>>& commands
//) {
//    std::cout << "Usage:\n";
//    std::cout << "  minmap [command] [options]\n";
//
//    std::cout << "Documentation:\n";
//    std::cout << "  https://minmap.github.io/\n";
//
//    std::cout << "Example usage:\n";
//    std::cout << "  minmap help [ -h, --help ]\n";
//    std::cout << "  minmap gui\n";
//    std::cout << "  minmap gui -h [ --help ]\n";
//    std::cout << "  minmap automatic_reconstructor -h [ --help ]\n";
//    std::cout << "  minmap automatic_reconstructor --image_path IMAGES "
//        "--workspace_path WORKSPACE\n";
//    std::cout << "  minmap feature_extractor --image_path IMAGES --database_path "
//        "DATABASE\n";
//    std::cout << "  minmap exhaustive_matcher --database_path DATABASE\n";
//    std::cout << "  minmap mapper --image_path IMAGES --database_path DATABASE "
//        "--output_path MODEL\n";
//    std::cout << "  ...\n";
//
//    std::cout << "Available commands:\n";
//    std::cout << "  help\n";
//    for (const auto& command : commands) {
//        std::cout << "  " << command.first << '\n';
//    }
//    std::cout << '\n';
//
//    return EXIT_SUCCESS;
//}
//
//
//int main(int argc, char** argv) {
//    colmap::InitializeGlog(argv);
//
//    std::vector<std::pair<std::string, command_func_t>> commands;
//    commands.emplace_back("bundle_adjuster", &minmap::RunBundleAdjuster);
//    commands.emplace_back("color_extractor", &minmap::RunColorExtractor);
//    commands.emplace_back("database_cleaner", &minmap::RunDatabaseCleaner);
//    commands.emplace_back("database_creator", &minmap::RunDatabaseCreator);
//    commands.emplace_back("database_merger", &minmap::RunDatabaseMerger);
//    commands.emplace_back("delaunay_mesher", &minmap::RunDelaunayMesher);
//    commands.emplace_back("exhaustive_matcher", &minmap::RunExhaustiveMatcher);
//    commands.emplace_back("feature_extractor", &minmap::RunFeatureExtractor);
//    commands.emplace_back("feature_importer", &minmap::RunFeatureImporter);
//    commands.emplace_back("hierarchical_mapper", &minmap::RunHierarchicalMapper);
//    commands.emplace_back("image_deleter", &minmap::RunImageDeleter);
//    commands.emplace_back("image_filterer", &minmap::RunImageFilterer);
//    commands.emplace_back("image_rectifier", &minmap::RunImageRectifier);
//    commands.emplace_back("image_registrator", &minmap::RunImageRegistrator);
//    commands.emplace_back("image_undistorter", &minmap::RunImageUndistorter);
//    commands.emplace_back("image_undistorter_standalone",
//        &minmap::RunImageUndistorterStandalone);
//    commands.emplace_back("mapper", &minmap::RunMapper);
//    commands.emplace_back("matches_importer", &minmap::RunMatchesImporter);
//    commands.emplace_back("model_aligner", &minmap::RunModelAligner);
//    commands.emplace_back("model_analyzer", &minmap::RunModelAnalyzer);
//    commands.emplace_back("model_comparer", &minmap::RunModelComparer);
//    commands.emplace_back("model_converter", &minmap::RunModelConverter);
//    commands.emplace_back("model_cropper", &minmap::RunModelCropper);
//    commands.emplace_back("model_merger", &minmap::RunModelMerger);
//    commands.emplace_back("model_orientation_aligner",
//        &minmap::RunModelOrientationAligner);
//    commands.emplace_back("model_splitter", &minmap::RunModelSplitter);
//    commands.emplace_back("model_transformer", &minmap::RunModelTransformer);
//    commands.emplace_back("patch_match_stereo", &minmap::RunPatchMatchStereo);
//    commands.emplace_back("point_filtering", &minmap::RunPointFiltering);
//    commands.emplace_back("point_triangulator", &minmap::RunPointTriangulator);
//    commands.emplace_back("pose_prior_mapper", &minmap::RunPosePriorMapper);
//    commands.emplace_back("poisson_mesher", &minmap::RunPoissonMesher);
//    commands.emplace_back("rig_configurator", &minmap::RunRigConfigurator);
//    commands.emplace_back("rig_bundle_adjuster", &minmap::RunRigBundleAdjuster);
//    commands.emplace_back("sequential_matcher", &minmap::RunSequentialMatcher);
//    commands.emplace_back("spatial_matcher", &minmap::RunSpatialMatcher);
//    commands.emplace_back("stereo_fusion", &minmap::RunStereoFuser);
//    commands.emplace_back("transitive_matcher", &minmap::RunTransitiveMatcher);
//    commands.emplace_back("vocab_tree_builder", &minmap::RunVocabTreeBuilder);
//    commands.emplace_back("vocab_tree_matcher", &minmap::RunVocabTreeMatcher);
//    commands.emplace_back("vocab_tree_retriever", &minmap::RunVocabTreeRetriever);
//
//    if (argc == 1) {
//        return ShowHelp(commands);
//    }
//
//    const std::string command = argv[1];
//    if (command == "help" || command == "-h" || command == "--help") {
//        return ShowHelp(commands);
//    }
//    else {
//        command_func_t matched_command_func = nullptr;
//        for (const auto& command_func : commands) {
//            if (command == command_func.first) {
//                matched_command_func = command_func.second;
//                break;
//            }
//        }
//        if (matched_command_func == nullptr) {
//            LOG(ERROR) << colmap::StringPrintf(
//                "Command `%s` not recognized. To list the "
//                "available commands, run `minmap help`.",
//                command.c_str());
//            return EXIT_FAILURE;
//        }
//        else {
//            int command_argc = argc - 1;
//            char** command_argv = &argv[1];
//            command_argv[0] = argv[0];
//            return matched_command_func(command_argc, command_argv);
//        }
//    }
//
//    return ShowHelp(commands);
//}