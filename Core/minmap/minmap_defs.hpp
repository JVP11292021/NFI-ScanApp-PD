#ifndef MINMAP_DEFS_H
#define MINMAP_DEFS_H

#define GLOG_NO_ABBREVIATED_SEVERITIES
#define GLOG_USE_GLOG_EXPORT
#include <glog/logging.h>
#include <colmap/util/logging.h>

#define MM_NS_B namespace minmap {
#define MM_NS_E }

MM_NS_B

#if defined(COLMAP_CUDA_ENABLED) || !defined(COLMAP_GUI_ENABLED)
const bool kUseOpenGL = false;
#else
const bool kUseOpenGL = true;
#endif

MM_NS_E

#endif // MINMAP_DEFS_H