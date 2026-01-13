package com.example.ipmedth_nfi.bridge

class NativeDescriptors {
    companion object {
        init {
            // Load your native library (make sure the library is compiled as libnative.so)
            System.loadLibrary("engineBackendBridge")
        }

        
    }
}