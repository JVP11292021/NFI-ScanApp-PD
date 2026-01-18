package com.example.ipmedth_nfi.bridge

import android.content.res.AssetManager
import android.view.Surface

class NativeAndroidEngine() {
    companion object {
        init {
            System.loadLibrary("renderEngine")
        }
    }

    private external fun nativeCreate(surface: Surface, assetManager: AssetManager)
    private external fun nativeDestroy()
    private external fun nativeResize(width: Int, height: Int)
    private external fun nativeDraw()

    fun create(surface: Surface, assetManager: AssetManager) {
        nativeCreate(surface, assetManager)
    }

    fun destroy() {
        nativeDestroy()
    }

    fun resize(width: Int, height: Int) {
        nativeResize(width, height)
    }

    fun draw() {
        nativeDraw()
    }

}