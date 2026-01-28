#pragma once

#include "logging.h"

#include <cstdio>
#include <cstdlib>
#include <string>

#include <sqlite/sqlite3.h>

namespace colmap {

inline int SQLite3CallHelper(int result_code,
                             const std::string& filename,
                             int line) {
  switch (result_code) {
    case SQLITE_OK:
    case SQLITE_ROW:
    case SQLITE_DONE:
      return result_code;
    default:
      LogMessageFatalThrow<std::runtime_error>(filename.c_str(), line).stream()
          << "SQLite error: " << sqlite3_errstr(result_code);
      return result_code;
  }
}

#define SQLITE3_CALL(func) colmap::SQLite3CallHelper(func, __FILE__, __LINE__)

#define SQLITE3_EXEC(database, sql, callback)                             \
  {                                                                       \
    char* err_msg = nullptr;                                              \
    const int result_code =                                               \
        sqlite3_exec(database, sql, callback, nullptr, &err_msg);         \
    if (result_code != SQLITE_OK) {                                       \
      LOG(MM_ERROR) << "SQLite error [" << __FILE__ << ", line " << __LINE__ \
                 << "]: " << err_msg;                                     \
      sqlite3_free(err_msg);                                              \
    }                                                                     \
  }

}  // namespace colmap
