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
            setSurfaceLifecycle(1) // SURFACE_LIFECYCLE_FOLLOWS_VISIBILITY
            holder.addCallback(object : SurfaceHolder.Callback {
                override fun surfaceCreated(holder: SurfaceHolder) {
                    // 1. Initialize engine with the actual surface
                    engine.initRenderEngine(
                        holder.surface,
                        assetManager,
                        width,
                        height
                    )
                    // 2. Start the RenderLoop thread
                    engine.start()
                }

                override fun surfaceChanged(
                    holder: SurfaceHolder,
                    format: Int,
                    width: Int,
                    height: Int
                ) {
                    Log.i("NFI", "Changed surface!");
                    // Handle orientation changes or resizing
                    engine.surfaceChanged(holder.surface)
                }

                override fun surfaceDestroyed(holder: SurfaceHolder) {
                    // 3. Stop the thread and cleanup before surface is gone
                    engine.stop()
                    engine.destroyRenderEngine()
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