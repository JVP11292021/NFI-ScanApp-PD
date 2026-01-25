package com.example.ipmedth_nfi.ui.components.vk

import android.util.Log
import android.view.SurfaceView
import android.view.SurfaceHolder
import androidx.compose.foundation.gestures.detectTapGestures
import androidx.compose.foundation.gestures.detectTransformGestures
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.viewinterop.AndroidView
import com.example.ipmedth_nfi.bridge.NativeAndroidEngine
import kotlin.math.abs

@Composable
fun VulkanSurface(
    modifier: Modifier = Modifier,
    engine: NativeAndroidEngine
) {
    val context = LocalContext.current
    val assetManager = context.assets

    // Track whether two fingers are active so single-finger pan can be suppressed
    var twoFingerActive by remember { mutableStateOf(false) }

    val surfaceView = SurfaceView(context).apply {
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

    AndroidView(
        factory = { surfaceView },
        modifier = modifier
            .pointerInput(Unit) {
                detectTapGestures(
                    onDoubleTap = { offset ->
                        Log.i("UI", "On double tap")
                        Log.i("DoubleTap", "x: ${offset.x}, y: ${offset.y}")
                        engine.draw()
                    },
                    onTap = { offset ->
                        Log.i("UI", "On tap")
                        Log.i("Tap", "x: ${offset.x}, y: ${offset.y}")
                        engine.onTap(offset.x, offset.y)
                        engine.draw()
                    }
                )
            }
            .pointerInput(Unit) {
                val touchSlop = 8f
                val scaleThreshold = 0.02f

                awaitPointerEventScope {
                    var previousCentroid: Offset? = null
                    var previousDistance: Float? = null

                    while (true) {
                        val event = awaitPointerEvent()
                        val pressed = event.changes.filter { it.pressed }

                        if (pressed.size == 2) {
                            twoFingerActive = true

                            // compute positions and centroid
                            val p0 = pressed[0].position
                            val p1 = pressed[1].position
                            val centroid = Offset((p0.x + p1.x) / 2f, (p0.y + p1.y) / 2f)

                            // compute distance between pointers for pinch scale
                            val dx = p1.x - p0.x
                            val dy = p1.y - p0.y
                            val distance = kotlin.math.hypot(dx, dy)

                            previousDistance?.let { prevDist ->
                                if (prevDist > 0f) {
                                    val scale = distance / prevDist
                                    if (abs(scale - 1f) >= scaleThreshold) {
                                        Log.i("TwoFingerPinch", "scale: $scale")
                                        engine.onPinch(scale)
                                        engine.draw()

                                        // consume so other handlers don't duplicate pinch handling
                                        pressed.forEach { it.consume() }
                                    }
                                }
                            }

                            previousCentroid?.let { prev ->
                                val delta = centroid - prev
                                if (delta.getDistance() >= touchSlop) {
                                    Log.i("TwoFingerDrag", "dx: ${delta.x}, dy: ${delta.y}")
                                    engine.onStrafe(delta.x, delta.y);
                                    engine.draw()

                                    // consume so detectTransformGestures (and others) won't also react to the two-finger motion
                                    pressed.forEach { it.consume() }
                                }
                            }

                            previousCentroid = centroid
                            previousDistance = distance
                        } else {
                            // reset trackers when not exactly two pointers
                            previousCentroid = null
                            previousDistance = null

                            twoFingerActive = false
                        }
                    }
                }
            }
            .pointerInput(Unit) {
                detectTransformGestures { _, pan, _, _ ->
                    // suppress single-finger pan when two fingers are active
                    if (twoFingerActive) return@detectTransformGestures

                    if (pan != Offset.Zero) {
                        Log.i("UI", "On pan")
                        engine.onDrag(pan.x, pan.y)
                        engine.draw()
                    }
                }
            }
        ,
        update = { }
    )
}