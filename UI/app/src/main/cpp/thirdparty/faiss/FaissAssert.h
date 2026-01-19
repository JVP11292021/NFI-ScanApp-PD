/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// -*- c++ -*-

#ifndef FAISS_ASSERT_INCLUDED
#define FAISS_ASSERT_INCLUDED

#include "FaissException.h"
#include <cstdlib>
#include <cstdio>
#include <string>

#if defined(__ANDROID__)
#include <android/log.h>
#endif

///
/// Assertions
///

#if defined(__ANDROID__)
#define TAG "faiss"
#define FAISS_LOGD(...) __android_log_print(android_LogPriority::ANDROID_LOG_DEBUG,TAG ,__VA_ARGS__)
#define FAISS_LOGI(...) __android_log_print(android_LogPriority::ANDROID_LOG_INFO,TAG ,__VA_ARGS__)
#define FAISS_LOGE(...) __android_log_print(android_LogPriority::ANDROID_LOG_ERROR,TAG ,__VA_ARGS__)
#else
#define FAISS_LOGD(...)
#define FAISS_LOGI(...)
#define FAISS_LOGE(...)
#endif


#define FAISS_ASSERT(X)                                                 \
  do {                                                                  \
    if (! (X)) {                                                        \
      fprintf(stderr, "Faiss assertion '%s' failed in %s "              \
               "at %s:%d\n",                                            \
               #X, __PRETTY_FUNCTION__, __FILE__, __LINE__);            \
      abort();                                                          \
    }                                                                   \
  } while (false)

#define FAISS_ASSERT_MSG(X, MSG)                                        \
  do {                                                                  \
    if (! (X)) {                                                        \
      fprintf(stderr, "Faiss assertion '%s' failed in %s "              \
               "at %s:%d; details: " MSG "\n",                          \
               #X, __PRETTY_FUNCTION__, __FILE__, __LINE__);            \
      abort();                                                          \
    }                                                                   \
  } while (false)

#define FAISS_ASSERT_FMT(X, FMT, ...)                                   \
  do {                                                                  \
    if (! (X)) {                                                        \
      fprintf(stderr, "Faiss assertion '%s' failed in %s "              \
               "at %s:%d; details: " FMT "\n",                          \
               #X, __PRETTY_FUNCTION__, __FILE__, __LINE__, __VA_ARGS__); \
      abort();                                                          \
    }                                                                   \
  } while (false)

///
/// Exceptions for returning user errors
///

#define FAISS_THROW_MSG(MSG)                                            \
  do {                                                                  \
    throw faiss::FaissException(MSG, __PRETTY_FUNCTION__, __FILE__, __LINE__); \
  } while (false)

#define FAISS_THROW_FMT(FMT, ...)                                       \
  do {                                                                  \
    std::string __s;                                                    \
    int __size = snprintf(nullptr, 0, FMT, __VA_ARGS__);                \
    __s.resize(__size + 1);                                             \
    snprintf(&__s[0], __s.size(), FMT, __VA_ARGS__);                    \
    FAISS_LOGE("%s",__s.c_str());                                              \
    throw faiss::FaissException(__s, __PRETTY_FUNCTION__, __FILE__, __LINE__); \
  } while (false)

///
/// Exceptions thrown upon a conditional failure
///

#define FAISS_THROW_IF_NOT(X)                           \
  do {                                                  \
    if (!(X)) {                                         \
      FAISS_THROW_FMT("Error: '%s' failed", #X);        \
    }                                                   \
  } while (false)

#define FAISS_THROW_IF_NOT_MSG(X, MSG)                  \
  do {                                                  \
    if (!(X)) {                                         \
      FAISS_THROW_FMT("Error: '%s' failed: " MSG, #X);  \
    }                                                   \
  } while (false)

#define FAISS_THROW_IF_NOT_FMT(X, FMT, ...)                             \
  do {                                                                  \
    if (!(X)) {                                                         \
      FAISS_THROW_FMT("Error: '%s' failed: " FMT, #X, __VA_ARGS__);     \
    }                                                                   \
  } while (false)

#endif
