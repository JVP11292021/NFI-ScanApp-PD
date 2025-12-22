#ifndef MINMAP_DEFS_H
#define MINMAP_DEFS_H

#if !defined(GLOG_NO_ABBREVIATED_SEVERITIES)
#define GLOG_NO_ABBREVIATED_SEVERITIES
#endif
#if !defined(GLOG_USE_GLOG_EXPORT)
#define GLOG_USE_GLOG_EXPORT
#endif
#include <glog/logging.h>
#include "colmap/util/logging.h"

#define MM_NS_B namespace minmap {
#define MM_NS_E }

MM_NS_B

#if defined(COLMAP_CUDA_ENABLED) || !defined(COLMAP_GUI_ENABLED)
const bool kUseOpenGL = true;
#else
const bool kUseOpenGL = true;
#endif

MM_NS_E

#endif // MINMAP_DEFS_H