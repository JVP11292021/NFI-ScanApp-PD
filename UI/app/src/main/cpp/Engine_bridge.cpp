//
// Created by jessy on 1/13/2026.
//

#include "AndroidEngine.h"
#include "RenderLoopProcess.h"

#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <EngineBackend/defs.hpp>


static RenderLoopProcess* process = nullptr;
static AndroidEngine* engine = nullptr;
static AAssetManager* asset_manager = nullptr;
static ANativeWindow* nav_window = nullptr;

static ANativeWindow * __winFromSurface(_JNIEnv *env, jobject surface);
static AAssetManager * __assetManagerFromJava(_JNIEnv *env, jobject manager);

extern "C"
JNIEXPORT jint JNICALL
Java_com_example_ipmedth_1nfi_bridge_NativeAndroidEngine_initRenderEngine(
        JNIEnv *env,
        jobject /* this */,
        jobject surface,
        jobject assetManager,
        jint width,
        jint height
) {
    if (engine) {
        VLE_LOGF("Cannot create a double instance of AndroidEngine!");
        return 0;
    }
    nav_window = __winFromSurface(env, surface);
    asset_manager = __assetManagerFromJava(env, assetManager);
//    VLE_LOGI("width=", std::to_string(width).c_str(), ", height=", std::to_string(height).c_str());

    if (!engine) {
        try {
            engine = new AndroidEngine(asset_manager, nav_window, width, height);
            VLE_LOGD("Engine instance created successfully!");
        } catch (std::runtime_error& ex) {
            VLE_LOGE(ex.what());
            return 0;
        }
    }

    VLE_LOGI("Vulkan instance loaded correctly!");
    return 1;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_example_ipmedth_1nfi_bridge_NativeAndroidEngine_surfaceChanged(
        JNIEnv *env,
        jobject /* this */,
        jobject surface
) {
    if (!engine) {
        VLE_LOGF("Could not activate window change detection 'AndroidEngine' undefined. Make sure to init the AndroidEngine!");
        return 0;
    }

    nav_window = __winFromSurface(env, surface);
    engine->resize(nav_window);

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

    if (process) {
        delete process;
        process = nullptr;
        VLE_LOGI("Destroyed the RenderLoopProcess instance!");
    }

    if (process || engine)
        VLE_LOGW("Was not able to destroy render instances!");
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_example_ipmedth_1nfi_bridge_NativeAndroidEngine_start(
        JNIEnv *env,
        jobject /* this */
) {
    if (!engine) {
        VLE_LOGE("Cannot start render loop, AndroidEngine was uninitialized!");
        return 0;
    }

    process = new RenderLoopProcess(engine);
    VLE_LOGI("Starting render loop!");
    return static_cast<jint>(process->start());     // Thread begins render loop
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_example_ipmedth_1nfi_bridge_NativeAndroidEngine_stop(
        JNIEnv *env,
        jobject /* this */
) {
    if (!process) {
        VLE_LOGF("Cannot stop the render loop process when it isn't running!");
        return 0;
    }

    process->stop();
    VLE_LOGI("Stopped the render loop process");
    return 1;
}

ANativeWindow * __winFromSurface(_JNIEnv *env, jobject surface) {
    return ANativeWindow_fromSurface(env, surface);
}

AAssetManager * __assetManagerFromJava(_JNIEnv *env, jobject manager) {
    return AAssetManager_fromJava(env, manager);
}