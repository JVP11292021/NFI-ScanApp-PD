
#ifndef IPMEDTH_NFI_ANDROID_LOG_H
#define IPMEDTH_NFI_ANDROID_LOG_H

#include <android/log.h>

#define TAG "ceres"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG ,__VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG ,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG ,__VA_ARGS__)

#endif //IPMEDTH_NFI_ANDROID_LOG_H
