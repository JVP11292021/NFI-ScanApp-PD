package com.example.ipmedth_nfi.ui.components.vk

import android.util.Log
import android.view.SurfaceView
import android.view.SurfaceHolder
import androidx.compose.runtime.Composable
import androidx.compose.runtime.remember
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.viewinterop.AndroidView
import com.example.ipmedth_nfi.bridge.NativeAndroidEngine

@Composable
fun VulkanSurface(
    modifier: Modifier = Modifier,
    engine: NativeAndroidEngine
) {
    val context = LocalContext.current
    val assetManager = context.assets

    val surfaceView = remember {
        SurfaceView(context).apply {
            // Ensure the surface lifecycle follows visibility for better resource management
//            setSurfaceLifecycle(1)

            holder.addCallback(object : SurfaceHolder.Callback2 {
                override fun surfaceCreated(holder: SurfaceHolder) {
                    // Initialize the native Vulkan engine with the provided surface
                    engine.create(holder.surface, assetManager)
                }

                override fun surfaceChanged(
                    holder: SurfaceHolder,
                    format: Int,
                    width: Int,
                    height: Int
                ) {
                    Log.i("NFI", "Resizing surface to: $width x $height")
                    // Notify the native backend of the new dimensions
                    engine.resize(width, height)
                }

                override fun surfaceDestroyed(holder: SurfaceHolder) {
                    // Clean up native resources before the surface is fully removed
                    engine.destroy()
                }

                override fun surfaceRedrawNeeded(holder: SurfaceHolder) {
                    // Trigger a frame draw when the system requests a redraw
                    engine.draw()
                }
            })
        }
    }

    AndroidView(
        modifier = modifier,
        factory = { surfaceView },
        update = { }
    )
}