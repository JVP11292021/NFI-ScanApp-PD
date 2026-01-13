//
// Created by rolan on 12/01/2026.
//


#include <jni.h>

extern "C"
JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM* vm, void*) {

    return JNI_VERSION_1_6;
}
