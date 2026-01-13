package com.example.ipmedth_nfi.bridge

class NativeAndroidEngine() {
    companion object {
        init {
            System.loadLibrary("renderEngine")
        }
    }

    public external fun initRenderEngine(surface: android.view.Surface, width: Int, height: Int)
}