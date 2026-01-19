package com.example.ipmedth_nfi.ui.components.vk

import android.util.Log
import android.view.SurfaceView
import android.view.SurfaceHolder
import androidx.compose.foundation.gestures.detectDragGestures
import androidx.compose.foundation.gestures.detectTapGestures
import androidx.compose.foundation.gestures.detectTransformGestures
import androidx.compose.runtime.Composable
import androidx.compose.runtime.remember
import androidx.compose.ui.Modifier
import androidx.compose.ui.input.pointer.pointerInput
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
        factory = { surfaceView },
        modifier = modifier
                .pointerInput(Unit) {
                    // Handle Taps and Double Taps
                    detectTapGestures(
                        onDoubleTap = { offset ->
                            // engine.onDoubleTap(offset.x, offset.y)
                        },
                        onTap = { offset ->
                            // engine.onTap(offset.x, offset.y)
                        }
                    )
                }
                .pointerInput(Unit) {
                    // Handle Dragging (Horizontal and Vertical)
                    detectDragGestures { change, dragAmount ->
                        change.consume()
                        // dragAmount.x and dragAmount.y tell you the delta moved
                        // engine.onDrag(dragAmount.x, dragAmount.y)
                    }
                }
                .pointerInput(Unit) {
                    // Handle Pinch (Zoom/Scale) and Rotation
                    detectTransformGestures { centroid, pan, zoom, rotation ->
                        // zoom is the scale factor (e.g., 1.1f for pinching out)
                        // engine.onScale(zoom)
                    }
                },
        update = { }
    )
}