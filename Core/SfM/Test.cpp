#include "Test.hpp"
//
//#include <cstdlib>
//#include <iostream>
//
//#define GLOG_NO_ABBREVIATED_SEVERITIES
//#define GLOG_USE_GLOG_EXPORT
//#include <glog/logging.h>
//#include <colmap/util/logging.h>
//#include <colmap/util/types.h>
//#include <colmap/controllers/option_manager.h>
//
//int test_main(int argc, char** argv) {
//	colmap::InitializeGlog(argv);
//
//	std::string message;
//	colmap::OptionManager options;
//	options.AddRequiredOption("message", &message);
//	
//	options.Parse(argc, argv);
//
//	std::cout << colmap::StringPrintf("Hello %s!\n", message.c_str());
//
//	return EXIT_SUCCESS;
//}