#include <jni.h>

//
// Created by jessy on 1/25/2026.
//

#include "ReconstructionEngine.hpp"

static minmap::ReconstructionEngine* engine = nullptr;

static std::string jstringToString(JNIEnv* env, jstring jstr);

extern "C"
JNIEXPORT void JNICALL
Java_com_example_ipmedth_1nfi_bridge_NativeReconstructionEngine_nativeCreate(
        JNIEnv* env,
        jobject /* thiz */,
        jstring dataset_path,
        jstring database_path
) {
    if (engine != nullptr) {
        delete engine;
        engine = nullptr;
    }

    std::string datasetPath = jstringToString(env, dataset_path);
    std::string databasePath = jstringToString(env, database_path);

    engine = new minmap::ReconstructionEngine(datasetPath, databasePath);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_ipmedth_1nfi_bridge_NativeReconstructionEngine_nativeDestroy(
        JNIEnv* /* env */,
        jobject /* thiz */
) {
    if (engine != nullptr) {
        delete engine;
        engine = nullptr;
    }
    else {
        LOG(MM_WARNING) << "Engine not initialized or already destroyed. Call create() first.";
    }
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_example_ipmedth_1nfi_bridge_NativeReconstructionEngine_nativeExtractMatchFeatures(
        JNIEnv* env,
        jobject /* thiz */,
        jint camera_mode,
        jstring descriptor_normalization,
        jstring image_list_path
) {
    if (engine == nullptr) {
        LOG(MM_ERROR) << "Engine not initialized. Call create() first.";
        return EXIT_FAILURE;
    }

    std::string descriptorNormalization =
            jstringToString(env, descriptor_normalization);
    std::string imageListPath =
            jstringToString(env, image_list_path);

    return static_cast<jint>(
            engine->extractFeatures(
                    camera_mode,
                    descriptorNormalization,
                    imageListPath
            )
    );
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_example_ipmedth_1nfi_bridge_NativeReconstructionEngine_nativeReconstruct(
        JNIEnv* env,
        jobject /* thiz */,
        jstring output_path,
        jstring input_path,
        jstring image_list_path,
        jboolean fix_existing_frames
) {
    if (engine == nullptr) {
        LOG(MM_ERROR) << "Engine not initialized. Call create() first.";
        return EXIT_FAILURE;
    }

    std::string outputPath = jstringToString(env, output_path);
    std::string inputPath = jstringToString(env, input_path);
    std::string imageListPath = jstringToString(env, image_list_path);

    return static_cast<jint>(
            engine->reconstruct(
                    outputPath,
                    inputPath,
                    imageListPath,
                    static_cast<bool>(fix_existing_frames)
            )
    );
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_example_ipmedth_1nfi_bridge_NativeReconstructionEngine_nativeMapModel(
        JNIEnv* env,
        jobject /* thiz */,
        jstring input_path,
        jstring output_path,
        jstring output_type,
        jboolean skip_distortion
) {
    if (engine == nullptr) {
        LOG(MM_ERROR) << "Engine not initialized. Call create() first.";
        return EXIT_FAILURE;
    }

    std::string inputPath = jstringToString(env, input_path);
    std::string outputPath = jstringToString(env, output_path);
    std::string outputType = jstringToString(env, output_type);

    return static_cast<jint>(
            engine->mapModel(
                    inputPath,
                    outputPath,
                    outputType,
                    static_cast<bool>(skip_distortion)
            )
    );
}

std::string jstringToString(JNIEnv* env, jstring jstr) {
    if (jstr == nullptr) throw std::runtime_error("Could not convert jstring to std::string");
    const char* chars = env->GetStringUTFChars(jstr, nullptr);
    std::string result(chars);
    env->ReleaseStringUTFChars(jstr, chars);
    return result;
}