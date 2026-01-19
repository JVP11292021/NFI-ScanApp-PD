#pragma once

#if defined(_WIN32) || defined(_WIN64)

#include "string.h"

#include <exception>
#include <iostream>

#include <glog/logging.h>

// Option checker macros. In contrast to glog, this function does not abort the
// program, but simply returns false on failure.
#define CHECK_OPTION_IMPL(expr) \
  colmap::__CheckOptionImpl(__FILE__, __LINE__, (expr), #expr)
#define CHECK_OPTION(expr)                                             \
  if (!colmap::__CheckOptionImpl(__FILE__, __LINE__, (expr), #expr)) { \
    return false;                                                      \
  }
#define CHECK_OPTION_OP(name, op, val1, val2)      \
  if (!colmap::__CheckOptionOpImpl(__FILE__,       \
                                   __LINE__,       \
                                   (val1 op val2), \
                                   val1,           \
                                   val2,           \
                                   #val1,          \
                                   #val2,          \
                                   #op)) {         \
    return false;                                  \
  }
#define CHECK_OPTION_EQ(val1, val2) CHECK_OPTION_OP(_EQ, ==, val1, val2)
#define CHECK_OPTION_NE(val1, val2) CHECK_OPTION_OP(_NE, !=, val1, val2)
#define CHECK_OPTION_LE(val1, val2) CHECK_OPTION_OP(_LE, <=, val1, val2)
#define CHECK_OPTION_LT(val1, val2) CHECK_OPTION_OP(_LT, <, val1, val2)
#define CHECK_OPTION_GE(val1, val2) CHECK_OPTION_OP(_GE, >=, val1, val2)
#define CHECK_OPTION_GT(val1, val2) CHECK_OPTION_OP(_GT, >, val1, val2)

// Alternative checks to throw an exception instead of aborting the program.
// Usage: THROW_CHECK(condition) << message;
//        THROW_CHECK_EQ(val1, val2) << message;
//        LOG(FATAL_THROW) << message;
// These macros are copied from glog/logging.h and extended to a new severity
// level FATAL_THROW.
#define COMPACT_GOOGLE_LOG_FATAL_THROW \
  colmap::LogMessageFatalThrowDefault(__FILE__, __LINE__)

#define LOG_TO_STRING_FATAL_THROW(message) \
  colmap::LogMessageFatalThrowDefault(__FILE__, __LINE__, message)

#define LOG_FATAL_THROW(exception) \
  colmap::LogMessageFatalThrow<exception>(__FILE__, __LINE__).stream()

#define THROW_CHECK(condition)                                       \
  LOG_IF(FATAL_THROW, GOOGLE_PREDICT_BRANCH_NOT_TAKEN(!(condition))) \
      << "Check failed: " #condition " "

#define THROW_CHECK_OP(name, op, val1, val2) \
  CHECK_OP_LOG(name, op, val1, val2, colmap::LogMessageFatalThrowDefault)

#define THROW_CHECK_EQ(val1, val2) THROW_CHECK_OP(_EQ, ==, val1, val2)
#define THROW_CHECK_NE(val1, val2) THROW_CHECK_OP(_NE, !=, val1, val2)
#define THROW_CHECK_LE(val1, val2) THROW_CHECK_OP(_LE, <=, val1, val2)
#define THROW_CHECK_LT(val1, val2) THROW_CHECK_OP(_LT, <, val1, val2)
#define THROW_CHECK_GE(val1, val2) THROW_CHECK_OP(_GE, >=, val1, val2)
#define THROW_CHECK_GT(val1, val2) THROW_CHECK_OP(_GT, >, val1, val2)

#define THROW_CHECK_NOTNULL(val) \
  colmap::ThrowCheckNotNull(     \
      __FILE__, __LINE__, "'" #val "' Must be non NULL", (val))

namespace colmap {

// Initialize glog at the beginning of the program.
void InitializeGlog(const char* argv);

////////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////////

const char* __GetConstFileBaseName(const char* file);

bool __CheckOptionImpl(const char* file,
                       int line,
                       bool result,
                       const char* expr_str);

template <typename T1, typename T2>
bool __CheckOptionOpImpl(const char* file,
                         const int line,
                         const bool result,
                         const T1& val1,
                         const T2& val2,
                         const char* val1_str,
                         const char* val2_str,
                         const char* op_str) {
  if (result) {
    return true;
  } else {
    LOG(ERROR) << StringPrintf("[%s:%d] Check failed: %s %s %s (%s vs. %s)",
                               __GetConstFileBaseName(file),
                               line,
                               val1_str,
                               op_str,
                               val2_str,
                               std::to_string(val1).c_str(),
                               std::to_string(val2).c_str());
    return false;
  }
}

inline std::string __MakeExceptionPrefix(const char* file, int line) {
  return "[" + std::string(__GetConstFileBaseName(file)) + ":" +
         std::to_string(line) + "] ";
}

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4722)
#endif

template <typename T>
class LogMessageFatalThrow : public google::LogMessage {
 public:
  LogMessageFatalThrow(const char* file, int line)
      : google::LogMessage(file, line, google::GLOG_ERROR, &message_),
        prefix_(__MakeExceptionPrefix(file, line)) {}
  LogMessageFatalThrow(const char* file, int line, std::string* message)
      : google::LogMessage(file, line, google::GLOG_ERROR, message),
        message_(*message),
        prefix_(__MakeExceptionPrefix(file, line)) {}
  LogMessageFatalThrow(const char* file,
                       int line,
                       const google::logging::internal::CheckOpString& result)

      : google::LogMessage(file, line, google::GLOG_ERROR, &message_),
        prefix_(__MakeExceptionPrefix(file, line)) {
    stream() << "Check failed: " << (*result.str_) << " ";
    // On LOG(FATAL) glog<0.7.0 does not bother cleaning up CheckOpString
    // so we do it here.
  }
  ~LogMessageFatalThrow() noexcept(false) {
    Flush();
#if defined(__cpp_lib_uncaught_exceptions) && \
    (__cpp_lib_uncaught_exceptions >= 201411L)
    if (std::uncaught_exceptions() == 0)
#else
    if (!std::uncaught_exception())
#endif
    {
      throw T(prefix_ + message_);
    }
  }

 private:
  std::string message_;
  std::string prefix_;
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

using LogMessageFatalThrowDefault = LogMessageFatalThrow<std::invalid_argument>;

template <typename T>
T ThrowCheckNotNull(const char* file, int line, const char* names, T&& t) {
  if (t == nullptr) {
    LogMessageFatalThrowDefault(file, line).stream() << names;
  }
  return std::forward<T>(t);
}

}  // namespace colmap

#elif defined(__ANDROID__)

#include "string.h"

#include <sstream>
#include <stdexcept>
#include <string>
#include <android/log.h>

#define ANDROID_LOG_TAG "COLMAP"
extern int g_verbosity;

// Option checker macros. In contrast to glog, this function does not abort the
// program, but simply returns false on failure.
#define CHECK_OPTION_IMPL(expr) \
  colmap::__CheckOptionImpl(__FILE__, __LINE__, (expr), #expr)

#define CHECK_OPTION(expr)                                             \
  if (!colmap::__CheckOptionImpl(__FILE__, __LINE__, (expr), #expr)) { \
    return false;                                                      \
  }

#define CHECK_OPTION_OP(name, op, val1, val2)      \
  if (!colmap::__CheckOptionOpImpl(__FILE__,       \
                                   __LINE__,       \
                                   (val1 op val2), \
                                   val1,           \
                                   val2,           \
                                   #val1,          \
                                   #val2,          \
                                   #op)) {         \
    return false;                                  \
  }
#define CHECK_OPTION_EQ(val1, val2) CHECK_OPTION_OP(_EQ, ==, val1, val2)
#define CHECK_OPTION_NE(val1, val2) CHECK_OPTION_OP(_NE, !=, val1, val2)
#define CHECK_OPTION_LE(val1, val2) CHECK_OPTION_OP(_LE, <=, val1, val2)
#define CHECK_OPTION_LT(val1, val2) CHECK_OPTION_OP(_LT, <, val1, val2)
#define CHECK_OPTION_GE(val1, val2) CHECK_OPTION_OP(_GE, >=, val1, val2)
#define CHECK_OPTION_GT(val1, val2) CHECK_OPTION_OP(_GT, >, val1, val2)

// Map glog severities from android
#define MM_INFO    android_LogPriority::ANDROID_LOG_INFO
#define MM_WARNING android_LogPriority::ANDROID_LOG_WARN
#define MM_ERROR   android_LogPriority::ANDROID_LOG_ERROR
#define MM_FATAL   android_LogPriority::ANDROID_LOG_FATAL

#define COMPACT_GOOGLE_LOG_FATAL_THROW \
  colmap::LogMessageFatalThrowDefault(__FILE__, __LINE__)

#define LOG_TO_STRING_FATAL_THROW(message) \
  colmap::LogMessageFatalThrowDefault(__FILE__, __LINE__, message)

#define LOG_FATAL_THROW(exception) \
  colmap::LogMessageFatalThrow<exception>(__FILE__, __LINE__).stream()

#define THROW_CHECK(condition)                                           \
  if (!(condition))                                                      \
    colmap::LogMessageFatalThrowDefault(__FILE__, __LINE__).stream()     \
        << "Check failed: " #condition " "

#define THROW_CHECK_OP(name, op, val1, val2)                              \
  if (!((val1) op (val2)))                                                \
    colmap::LogMessageFatalThrowDefault(__FILE__, __LINE__).stream()     \
        << "Check failed: " #val1 " " #op " " #val2 " ("                  \
        << (val1) << " vs. " << (val2) << ") "

#define THROW_CHECK_EQ(val1, val2) THROW_CHECK_OP(_EQ, ==, val1, val2)
#define THROW_CHECK_NE(val1, val2) THROW_CHECK_OP(_NE, !=, val1, val2)
#define THROW_CHECK_LE(val1, val2) THROW_CHECK_OP(_LE, <=, val1, val2)
#define THROW_CHECK_LT(val1, val2) THROW_CHECK_OP(_LT, <, val1, val2)
#define THROW_CHECK_GE(val1, val2) THROW_CHECK_OP(_GE, >=, val1, val2)
#define THROW_CHECK_GT(val1, val2) THROW_CHECK_OP(_GT, >, val1, val2)

#define THROW_CHECK_NOTNULL(val) \
  colmap::ThrowCheckNotNull(__FILE__, __LINE__, "'" #val "' Must be non NULL", (val))

// glog-style LOG() macro
#define LOG(severity) \
  colmap::AndroidLogMessage(severity, __FILE__, __LINE__).stream()

#define VLOG(level) LOG(MM_INFO)

namespace colmap {

// ----------------- Base Fatal Logger -----------------

class LogMessageFatalThrowDefault {
public:
    LogMessageFatalThrowDefault(const char* file, int line)
            : file_(file), line_(line) {}

    LogMessageFatalThrowDefault(const char* file, int line,
                                const std::string& msg)
            : file_(file), line_(line)
    {
        stream_ << msg;
    }

    ~LogMessageFatalThrowDefault() noexcept(false) {
        Throw();
    }

    std::ostringstream& stream() { return stream_; }

protected:
    void Throw() {
        std::string msg = Format();
        __android_log_print(ANDROID_LOG_FATAL, ANDROID_LOG_TAG, "%s", msg.c_str());
        throw std::runtime_error(msg);
    }

    std::string Format() const {
        std::ostringstream ss;
        ss << file_ << ":" << line_ << ": " << stream_.str();
        return ss.str();
    }

    const char* file_;
    int line_;
    std::ostringstream stream_;
};

// ----------------- Typed Fatal Logger -----------------

template <typename Exception>
class LogMessageFatalThrow {
public:
    LogMessageFatalThrow(const char* file, int line)
            : file_(file), line_(line) {}

    ~LogMessageFatalThrow() noexcept(false) {
        Throw();
    }

    std::ostringstream& stream() { return stream_; }

private:
    void Throw() {
        std::string msg = Format();
        __android_log_print(ANDROID_LOG_FATAL, ANDROID_LOG_TAG, "%s", msg.c_str());
        throw Exception(msg);
    }

    std::string Format() const {
        std::ostringstream ss;
        ss << file_ << ":" << line_ << ": " << stream_.str();
        return ss.str();
    }

    const char* file_;
    int line_;
    std::ostringstream stream_;
};

// ----------------- Android log message appender -----------------
class AndroidLogMessage {
public:
    AndroidLogMessage(int prio,
                      const char* file,
                      int line)
            : prio_(prio), file_(file), line_(line) {}

    ~AndroidLogMessage() {
        std::ostringstream final;
        final << file_ << ":" << line_ << ": " << stream_.str();
        __android_log_print(prio_, ANDROID_LOG_TAG, "%s", final.str().c_str());
    }

    std::ostringstream& stream() { return stream_; }

private:
    int prio_;
    const char* file_;
    int line_;
    std::ostringstream stream_;
};

// ----------------- NotNull Check -----------------

template <typename T>
T ThrowCheckNotNull(const char* file,
                    int line,
                    const char* expr,
                    T ptr
) {
    if (ptr == nullptr) {
        std::ostringstream ss;
        ss << file << ":" << line << ": " << expr;
        std::string msg = ss.str();
        __android_log_print(ANDROID_LOG_FATAL, ANDROID_LOG_TAG, "%s", msg.c_str());
        throw std::runtime_error(msg);
    }
    return ptr;
}

const char* __GetConstFileBaseName(const char* file);

bool __CheckOptionImpl(const char* file,
                       const int line,
                       const bool result,
                       const char* expr_str);

template <typename T1, typename T2>
bool __CheckOptionOpImpl(const char* file,
                         const int line,
                         const bool result,
                         const T1& val1,
                         const T2& val2,
                         const char* val1_str,
                         const char* val2_str,
                         const char* op_str
) {
    if (result) {
        return true;
    } else {
        LOG(MM_ERROR) << StringPrintf("[%s:%d] Check failed: %s %s %s (%s vs. %s)",
                                      __GetConstFileBaseName(file),
                                      line,
                                      val1_str,
                                      op_str,
                                      val2_str,
                                      std::to_string(val1).c_str(),
                                      std::to_string(val2).c_str());
        return false;
    }
}

}  // namespace colmap

#endif
