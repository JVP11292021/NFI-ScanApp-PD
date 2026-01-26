package com.example.ipmedth_nfi.pages.model

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
import com.example.ipmedth_nfi.bridge.NativeAndroidEngine
import com.example.ipmedth_nfi.ui.vk.VulkanRenderer
import com.example.ipmedth_nfi.viewmodel.SessionViewModel
import kotlin.math.abs
import kotlin.math.hypot

@Composable
fun Annotation(
    viewModel: SessionViewModel,
    modifier: Modifier = Modifier,
    engine: NativeAndroidEngine,
    projectDirPath: String? = null,
    actionId: String? = "000000"
) {
    // Track whether two fingers are active so single-finger pan can be suppressed
    var twoFingerActive by remember { mutableStateOf(false) }

    VulkanRenderer(
        engine = engine,
        projectDirPath = projectDirPath,
        actionId = actionId,
        modifier = modifier
            .pointerInput(Unit) {
                detectTapGestures(
                    onDoubleTap = { offset ->
                        engine.onTap(offset.x, offset.y)
                        engine.draw()           // 1st draw to place new marker
                        engine.draw()           // 2nd draw to refresh view
                    },
                    onTap = { offset ->
                        engine.onDoubleTap(offset.x, offset.y)
                        engine.draw()           // 1st draw to remove marker
                        engine.draw()           // 2nd draw to refresh view
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
                            val distance = hypot(dx, dy)

                            previousDistance?.let { prevDist ->
                                if (prevDist > 0f) {
                                    val scale = distance / prevDist
                                    if (abs(scale - 1f) >= scaleThreshold) {
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
                                    engine.onStrafe(delta.x, delta.y)
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
                        engine.onDrag(pan.x, pan.y)
                        engine.draw()
                    }
                }
            }
    )
}

