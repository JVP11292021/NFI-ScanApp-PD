package com.example.ipmedth_nfi.ui.vk

import android.util.Log
import android.view.SurfaceHolder
import android.view.SurfaceView
import androidx.compose.runtime.Composable
import androidx.compose.runtime.remember
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.viewinterop.AndroidView
import com.example.ipmedth_nfi.bridge.NativeAndroidEngine

@Composable
fun VulkanRenderer(
    engine: NativeAndroidEngine,
    projectDirPath: String? = null,
    actionId: String? = null,
    modifier: Modifier = Modifier,
    onEngineReady: (() -> Unit)? = null
) {
    val context = LocalContext.current
    val assetManager = context.assets

    val surfaceView = remember {
        SurfaceView(context).apply {
            holder.addCallback(object : SurfaceHolder.Callback2 {
                override fun surfaceCreated(holder: SurfaceHolder) {
                    // Initialize the native Vulkan engine with the provided surface
                    engine.create(holder.surface, assetManager, projectDirPath, actionId)
                    // Notify that engine is ready
                    onEngineReady?.invoke()
                }

                override fun surfaceChanged(
                    holder: SurfaceHolder,
                    format: Int,
                    width: Int,
                    height: Int
                ) {
                    Log.i("NFI", "Resizing surface to: $width x $height")
                    engine.resize(width, height)
                }

                override fun surfaceDestroyed(holder: SurfaceHolder) {
                    engine.destroy()
                }

                override fun surfaceRedrawNeeded(holder: SurfaceHolder) {
                    engine.draw()
                }
            })
        }
    }

    AndroidView(
        factory = { surfaceView },
        modifier = modifier,
        update = { }
    )
}
