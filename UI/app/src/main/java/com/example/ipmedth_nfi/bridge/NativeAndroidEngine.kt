package com.example.ipmedth_nfi.bridge

import android.content.res.AssetManager

class NativeAndroidEngine() {
    companion object {
        init {
            System.loadLibrary("renderEngine")
        }
    }

    public external fun initRenderEngine(surface: android.view.Surface, assetManager: AssetManager, width: Int, height: Int) : Int
    public external fun destroyRenderEngine()
    public external fun surfaceChanged(surface: android.view.Surface, width: Int, height: Int) : Int
    public external fun start() : Int
    public external fun stop() : Int

}