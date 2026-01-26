#ifndef IPMEDTH_NFI_LOGGING_H
#define IPMEDTH_NFI_LOGGING_H

// THis is a dummy logging file

// Dummy class to swallow the stream operators "<<"
struct NullLogger {
    template<typename T>
    NullLogger& operator<<(const T&) { return *this; }
};

// This helper ensures the code is parsed (for syntax errors)
// but never actually executed by the CPU.
#define NFI_DUMMY_LOG while(false) NullLogger()

// --- Core LOG Macros ---
#define LOG(severity)     NFI_DUMMY_LOG
#define VLOG(verboselevel) NFI_DUMMY_LOG
#define LOG_IF(severity, condition) NFI_DUMMY_LOG
#define DLOG(severity)    NFI_DUMMY_LOG

// Log Severities (needed if they are used as LOG(INFO))
#define INFO    0
#define WARNING 1
#define ERROR   2
#define FATAL   3

// --- CHECK Macros ---
// These usually take a condition, so we just ignore it.
#define CHECK(condition)  NFI_DUMMY_LOG
#define CHECK_EQ(a, b)    NFI_DUMMY_LOG
#define CHECK_NE(a, b)    NFI_DUMMY_LOG
#define CHECK_LE(a, b)    NFI_DUMMY_LOG
#define CHECK_LT(a, b)    NFI_DUMMY_LOG
#define CHECK_GE(a, b)    NFI_DUMMY_LOG
#define CHECK_GT(a, b)    NFI_DUMMY_LOG
#define CHECK_NOTNULL(x)  (x) // Returns the value so code stays functional


#endif //IPMEDTH_NFI_LOGGING_H
