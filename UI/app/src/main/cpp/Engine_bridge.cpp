//
// Created by jessy on 1/13/2026.
//

#include <jni.h>
#include <EngineBackend/defs.hpp>

#include "AndroidEngine.h"

static AndroidEngine* engine = nullptr;
static ANativeWindow* nav_window = nullptr;

extern "C"
JNIEXPORT jint JNICALL
Java_com_example_ipmedth_1nfi_bridge_NativeAndroidEngine_initRenderEngine(
        JNIEnv *env,
        jobject /* this */,
        jobject surface,
        jint width,
        jint height
) {
    nav_window = ANativeWindow_fromSurface(env, surface);
//    VLE_LOGI("width=", std::to_string(width).c_str(), ", height=", std::to_string(height).c_str());

    if (!engine) {
        try {
            engine = new AndroidEngine(nav_window, width, height);
            VLE_LOGD("Engine instance created successfully!");
        } catch (std::runtime_error& ex) {
            VLE_LOGE(ex.what());
            return 0;
        }
    }
    else {
        VLE_LOGF("Cannot create a double instance of AndroidEngine!");
        return 0;
    }

    if (!engine) {
        VLE_LOGE("Failed to create Vulkan instance!");
        return 0;
    }

    VLE_LOGI("Vulkan instance loaded correctly!");
    return 1;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_example_ipmedth_1nfi_bridge_NativeAndroidEngine_surfaceChanged(
        JNIEnv *env,
        jobject /* this */,
        jobject surface,
        jint width,
        jint height
) {
    if (!engine) {
        VLE_LOGF("Could not activate window change detection 'AndroidEngine' undefined. Make sure to init the AndroidEngine!");
        return 0;
    }

    engine->resize(width, height);
    return 1;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_ipmedth_1nfi_bridge_NativeAndroidEngine_destroyRenderEngine(
        JNIEnv *env,
        jobject /* this */
) {
    if (engine) {
        delete engine;
        engine = nullptr;
        VLE_LOGI("Destroyed the AndroidEngine instance!");
    }

    VLE_LOGW("Was not able to destroy AndroidEngine instance!");
}

