//
// Created by jessy on 1/13/2026.
//

#include <jni.h>
#include "EngineBackend/SwapChain.hpp"

JNIEXPORT jint JNICALL
Java_com_example_ipmedth_1nfi_bridge_NativeConstants_getMaxFramesInFlight(JNIEnv* env, jclass clazz) {
    return vle::EngineSwapChain::MAX_FRAMES_IN_FLIGHT;
}

