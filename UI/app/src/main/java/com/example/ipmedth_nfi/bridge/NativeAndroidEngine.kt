package com.example.ipmedth_nfi.bridge

import android.content.res.AssetManager
import android.view.Surface
import com.example.ipmedth_nfi.data.export.ProjectStorageManager
import com.example.ipmedth_nfi.model.Onderzoek

class NativeAndroidEngine() {
    companion object {
        init {
            System.loadLibrary("renderEngine")
        }
    }

    private external fun nativeCreate(surface: Surface, assetManager: AssetManager, projectDirPath: String?)
    private external fun nativeDestroy()
    private external fun nativeResize(width: Int, height: Int)
    private external fun nativeDraw()
    private external fun nativeOnDrag(deltaX: Float, deltaY: Float)
    private external fun nativeOnStrafe(deltaX: Float, deltaY: Float)
    private external fun nativeOnPinch(scaleFactor: Float)
    private external fun nativeOnTap(x: Float, y: Float)
    private external fun nativeOnDoubleTap(x: Float, y: Float)

    fun create(surface: Surface, assetManager: AssetManager, projectDirPath: String? = null) {
        nativeCreate(surface, assetManager, projectDirPath)
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

    fun onDrag(deltaX: Float, deltaY: Float) {
        nativeOnDrag(deltaX, deltaY)
    }

    fun onPinch(scaleFactor: Float) {
        nativeOnPinch(scaleFactor)
    }

    fun onStrafe(deltaX: Float, deltaY: Float) {
        nativeOnStrafe(deltaX, deltaY)
    }

    fun onTap(x: Float, y: Float) {
        nativeOnTap(x, y)
    }

    fun onDoubleTap(x: Float, y: Float) {
        nativeOnDoubleTap(x, y)
    }

}

