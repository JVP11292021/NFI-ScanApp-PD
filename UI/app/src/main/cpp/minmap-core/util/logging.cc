#include "logging.h"

#if defined(_WIN32) || defined(_WIN64)

namespace colmap {

void InitializeGlog(const char* argv) {
#ifndef _MSC_VER  // Broken in MSVC
  google::InstallFailureSignalHandler();
#endif
  google::InitGoogleLogging(argv);
}

const char* __GetConstFileBaseName(const char* file) {
  const char* base = strrchr(file, '/');
  if (!base) {
    base = strrchr(file, '\\');
  }
  return base ? (base + 1) : file;
}

bool __CheckOptionImpl(const char* file,
                       const int line,
                       const bool result,
                       const char* expr_str) {
  if (result) {
    return true;
  } else {
    LOG(ERROR) << StringPrintf("[%s:%d] Check failed: %s",
                               __GetConstFileBaseName(file),
                               line,
                               expr_str);
    return false;
  }
}

}  // namespace colmap

#elif defined(__ANDROID__)

namespace colmap {

int g_verbosity = 0;

const char* __GetConstFileBaseName(const char* file) {
    const char* base = strrchr(file, '/');
    if (!base) {
        base = strrchr(file, '\\');
    }
    return base ? (base + 1) : file;
}


bool __CheckOptionImpl(const char* file,
                       const int line,
                       const bool result,
                       const char* expr_str) {
    if (result) {
        return true;
    } else {
        LOG(MM_ERROR) << StringPrintf("[%s:%d] Check failed: %s",
                                   __GetConstFileBaseName(file),
                                   line,
                                   expr_str);
        return false;
    }
}

}

#endif