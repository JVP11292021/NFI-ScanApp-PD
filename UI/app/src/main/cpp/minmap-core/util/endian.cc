#include "endian.h"

#if defined(_WIN32) || defined(_WIN64)
#include <boost/predef/other/endian.h>
#endif

namespace colmap {

bool IsLittleEndian() {
#if defined(_WIN32) || defined(_WIN64)
#if BOOST_ENDIAN_LITTLE_BYTE
  return true;
#elif BOOST_ENDIAN_LITTLE_WORD
  // We do not support such exotic architectures.
  abort()
#else
  return false;
#endif
#elif defined(__ANDROID__)
  return true; // Embedded has no big endian for android devices
#endif
}

bool IsBigEndian() {
#if defined(_WIN32) || defined(_WIN64)
#if BOOST_ENDIAN_BIG_BYTE
  return true;
#elif BOOST_ENDIAN_BIG_WORD
  // We do not support such exotic architectures.
  abort()
#else
  return false;
#endif
#elif defined(__ANDROID__)
    return false; // Embedded has no big endian for android devices
#endif
}

}  // namespace colmap
