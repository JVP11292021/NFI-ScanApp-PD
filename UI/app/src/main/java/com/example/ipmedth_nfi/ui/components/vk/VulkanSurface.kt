package com.example.ipmedth_nfi.ui.components.vk

import android.util.Log
import android.view.SurfaceView
import android.view.SurfaceHolder
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.viewinterop.AndroidView
import com.example.ipmedth_nfi.bridge.NativeAndroidEngine

@Composable
fun VulkanSurface(
    modifier: Modifier = Modifier,
    engine: NativeAndroidEngine
) {
    AndroidView(
        modifier = modifier,
        factory = { context ->
            SurfaceView(context).apply {
                holder.addCallback(object : SurfaceHolder.Callback {
                    override fun surfaceCreated(holder: SurfaceHolder) {
                        Log.i("NFI, ", "Device width=$width, height=$height")
                        engine.initRenderEngine(
                            holder.surface,
                            width,
                            height
                        )
                    }

                    override fun surfaceChanged(
                        holder: SurfaceHolder,
                        format: Int,
                        width: Int,
                        height: Int
                    ) {
                        engine.initRenderEngine(
                            holder.surface,
                            width,
                            height
                        )
                    }

                    override fun surfaceDestroyed(holder: SurfaceHolder) {
                        // later: engine.destroy()
                    }
                })
            }
        }
    )
}
