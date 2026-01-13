//
// Created by jessy on 1/13/2026.
//

#include <jni.h>
#include <EngineBackend/defs.hpp>

#include "AndroidEngine.h"

static AndroidEngine* engine = nullptr;
static ANativeWindow* nav_window = nullptr;

extern "C"
JNIEXPORT void JNICALL
Java_com_example_ipmedth_1nfi_bridge_NativeAndroidEngine_initRenderEngine(
        JNIEnv *env,
        jobject /* this */,
        jobject surface,
        jint width,
        jint height
) {
    nav_window = ANativeWindow_fromSurface(env, surface);
    VLE_LOGI("width=", std::to_string(width).c_str(), ", height=", std::to_string(height).c_str());

    if (!engine) {
        try {
            engine = new AndroidEngine(nav_window, width, height);
        } catch (std::runtime_error& ex) {
            VLE_LOGE(ex.what());
            return;
        }
    }
    else {
        engine->resize(width, height);;
    }

    if (!engine) {
        VLE_LOGE("Failed to create Vulkan instance!");
    } else {
        VLE_LOGI("Vulkan instance created successfully!");
    }
}




