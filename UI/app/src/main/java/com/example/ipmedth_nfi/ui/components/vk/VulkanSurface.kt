package com.example.ipmedth_nfi.ui.components.vk

import android.view.SurfaceView
import android.view.SurfaceHolder
import androidx.compose.runtime.Composable
import androidx.compose.ui.viewinterop.AndroidView

@Composable
fun VulkanSurface(initVulkanSurface: (surface: android.view.Surface) -> Unit) {
    AndroidView(factory = { context ->
        SurfaceView(context).apply {
            holder.addCallback(object : SurfaceHolder.Callback {
                override fun surfaceCreated(holder: SurfaceHolder) {
                    initVulkanSurface(holder.surface)
                }
                override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {}
                override fun surfaceDestroyed(holder: SurfaceHolder) {}
            })
        }
    })
}
