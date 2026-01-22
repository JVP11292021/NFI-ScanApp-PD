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
    private external fun nativeOnDrag(deltaX: Float, deltaY: Float)
    private external fun nativeOnStrafe(deltaX: Float, deltaY: Float)
    private external fun nativeOnPinch(scaleFactor: Float)
    private external fun nativeOnTap(x: Float, y: Float)
//    private external fun nativeInitMarkerManager(markersBasePath: String)

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

//    fun initMarkerManager(onderzoek: Onderzoek) {
//        val markersBasePath = onderzoek.basepath
//        nativeInitMarkerManager(markersBasePath)
//    }
}