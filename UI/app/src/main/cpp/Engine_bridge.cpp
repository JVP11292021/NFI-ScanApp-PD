#include "AndroidEngine.h"

#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/native_window_jni.h>
#include <EngineBackend/defs.hpp>

static AndroidEngine* engineApp = nullptr;
static ANativeWindow* currentWindow = nullptr;

extern "C"
JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    VLE_LOGV("Loading VulkanAppBridge form JNI_onLoad");
    return JNI_VERSION_1_6;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_ipmedth_1nfi_bridge_NativeAndroidEngine_nativeCreate(
        JNIEnv *env,
        jobject vulkanAppBridge,
        jobject surface,
        jobject pAssetManager,
        jstring projectDirPath
) {
    if (engineApp) {
        delete engineApp;
        engineApp = nullptr;
    }

    if (currentWindow) {
        ANativeWindow_release(currentWindow);
        currentWindow = nullptr;
    }

    auto window = ANativeWindow_fromSurface(env, surface);
    auto assetManager = AAssetManager_fromJava(env, pAssetManager);
    if (!window || !assetManager) {
        VLE_LOGF("Was unable to initialize the ANativeWindow and/or the AAssetManager from UI!");
        return;
    }

    currentWindow = window;

    const char* projectPath = nullptr;
    if (projectDirPath != nullptr) {
        projectPath = env->GetStringUTFChars(projectDirPath, nullptr);
    }

    try {
        engineApp = new AndroidEngine(
                assetManager, window, ANativeWindow_getWidth(window), ANativeWindow_getHeight(window), projectPath);
    } catch (std::runtime_error& er) {
        VLE_LOGF(er.what());
        if (projectPath) {
            env->ReleaseStringUTFChars(projectDirPath, projectPath);
        }
        return;
    }

    if (projectPath) {
        env->ReleaseStringUTFChars(projectDirPath, projectPath);
    }

    VLE_LOGD("AndroidEngine app instance created successfully!");
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_ipmedth_1nfi_bridge_NativeAndroidEngine_nativeDestroy(JNIEnv *env, jobject vulkanAppBridge) {
    if (engineApp) {
        delete engineApp;
        engineApp = nullptr;
        VLE_LOGI("Destroyed the AndroidEngine app instance!");
    }

    if (currentWindow) {
        ANativeWindow_release(currentWindow);
        currentWindow = nullptr;
        VLE_LOGI("Released ANativeWindow reference!");
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_ipmedth_1nfi_bridge_NativeAndroidEngine_nativeResize(JNIEnv *env, jobject vulkanAppBridge, jint width, jint height) {
    VLE_LOGD("Resized surface to: ", std::to_string(width).c_str(), "x", std::to_string(height).c_str());
    if (engineApp) {
        engineApp->resize(width, height);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_ipmedth_1nfi_bridge_NativeAndroidEngine_nativeDraw(JNIEnv *env, jobject vulkanAppBridge) {
    if (engineApp) {
        engineApp->drawFrame();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_ipmedth_1nfi_bridge_NativeAndroidEngine_nativeOnDrag(JNIEnv *env, jobject thiz,
                                                                      jfloat delta_x,
                                                                      jfloat delta_y) {
    if(engineApp) {
        engineApp->onDrag(delta_x, delta_y);
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_ipmedth_1nfi_bridge_NativeAndroidEngine_nativeOnPinch(JNIEnv *env, jobject thiz,
                                                                       jfloat scale_factor) {
    if(engineApp) {
        engineApp->onZoom(scale_factor);
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_ipmedth_1nfi_bridge_NativeAndroidEngine_nativeOnStrafe(JNIEnv *env, jobject thiz,
                                                                        jfloat delta_x,
                                                                        jfloat delta_y) {
    if(engineApp) {
        engineApp->onStrafe(-delta_x, -delta_y);
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_ipmedth_1nfi_bridge_NativeAndroidEngine_nativeOnTap(JNIEnv *env, jobject thiz,
                                                                     jfloat x, jfloat y) {
    if (engineApp) {
        engineApp->onTap(static_cast<uint32_t>(x), static_cast<uint32_t>(y));
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_ipmedth_1nfi_bridge_NativeAndroidEngine_nativeOnDoubleTap(JNIEnv *env, jobject thiz,
                                                                           jfloat x, jfloat y) {
    if (engineApp) {
        engineApp->onDoubleTap(static_cast<uint32_t>(x), static_cast<uint32_t>(y));
    }
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_ipmedth_1nfi_bridge_NativeAndroidEngine_nativeGetProjectDirPath(JNIEnv *env,
                                                                                 jobject thiz,
                                                                                 jobject storage_manager,
                                                                                 jobject onderzoek) {
    jclass storageClass = env->GetObjectClass(storage_manager);
    if (!storageClass) {
        return nullptr;
    }

    jmethodID getPathMethod = env->GetMethodID(
            storageClass,
            "getProjectDirPath",
            "(Lcom/example/ipmedth_nfi/model/Onderzoek;)Ljava/lang/String;"
    );

    if (!getPathMethod) {
        return nullptr;
    }

    jstring path = (jstring) env->CallObjectMethod(
            storage_manager,
            getPathMethod,
            onderzoek
    );

    return path;

}