package com.example.ipmedth_nfi.pages.model

import androidx.compose.animation.AnimatedVisibility
import androidx.compose.foundation.gestures.detectTransformGestures
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Settings
import androidx.compose.material.icons.filled.Close
import androidx.compose.material3.Button
import androidx.compose.material3.Card
import androidx.compose.material3.FloatingActionButton
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Slider
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.unit.dp
import com.example.ipmedth_nfi.bridge.NativeAndroidEngine
import com.example.ipmedth_nfi.ui.vk.VulkanRenderer
import com.example.ipmedth_nfi.viewmodel.SessionViewModel
import kotlin.math.PI
import kotlin.math.abs
import kotlin.math.hypot

@Composable
fun ModelPage(
    viewModel: SessionViewModel,
    modifier: Modifier = Modifier,
    engine: NativeAndroidEngine,
    projectDirPath: String? = null,
    actionId: String? = "01"
) {
    // Track whether two fingers are active so single-finger pan can be suppressed
    var twoFingerActive by remember { mutableStateOf(false) }

    // Initial rotation values from AndroidEngine (in radians)
    val initialRotationX = Math.toRadians(9.0).toFloat()
    val initialRotationY = Math.toRadians(180.0).toFloat()
    val initialRotationZ = Math.toRadians(93.0).toFloat()

    // Rotation offset from initial values (starts at saved values from roomModel, or 0 if none)
    val savedRotation = viewModel.roomModel
    var rotationOffsetX by remember { mutableStateOf(savedRotation?.rotationOffsetX ?: 0f) }
    var rotationOffsetY by remember { mutableStateOf(savedRotation?.rotationOffsetY ?: 0f) }
    var rotationOffsetZ by remember { mutableStateOf(savedRotation?.rotationOffsetZ ?: 0f) }

    // Menu visibility state
    var showRotationMenu by remember { mutableStateOf(false) }

    // Reset function
    val resetRotation = {
        rotationOffsetX = 0f
        rotationOffsetY = 0f
        rotationOffsetZ = 0f
        engine.onRotate(initialRotationX, initialRotationY, initialRotationZ)
        engine.draw()
        viewModel.updateRoomModelRotation(0f, 0f, 0f)
    }

    // Apply saved rotation to engine when page loads
    LaunchedEffect(Unit) {
        engine.onRotate(
            initialRotationX + rotationOffsetX,
            initialRotationY + rotationOffsetY,
            initialRotationZ + rotationOffsetZ
        )
        engine.draw()
    }

    Box(modifier = modifier) {
        VulkanRenderer(
            engine = engine,
            projectDirPath = projectDirPath,
            actionId = actionId,
            modifier = Modifier
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

        // Floating action button to toggle rotation menu
        FloatingActionButton(
            onClick = { showRotationMenu = !showRotationMenu },
            modifier = Modifier
                .align(Alignment.BottomEnd)
                .padding(16.dp)
        ) {
            Icon(
                imageVector = if (showRotationMenu) Icons.Default.Close else Icons.Default.Settings,
                contentDescription = if (showRotationMenu) "Close rotation controls" else "Open rotation controls"
            )
        }

        // Collapsible rotation control menu
        AnimatedVisibility(
            visible = showRotationMenu,
            modifier = Modifier
                .align(Alignment.BottomEnd)
                .padding(16.dp)
        ) {
            Card(
                modifier = Modifier.fillMaxWidth(0.85f)
            ) {
                Column(
                    modifier = Modifier.padding(16.dp)
                ) {
                    Row(
                        verticalAlignment = Alignment.CenterVertically,
                        modifier = Modifier.fillMaxWidth()
                    ) {
                        Text(
                            text = "Rotation Controls",
                            style = MaterialTheme.typography.titleMedium,
                            modifier = Modifier.weight(1f)
                        )
                        IconButton(onClick = { showRotationMenu = false }) {
                            Icon(Icons.Default.Close, contentDescription = "Close")
                        }
                    }

                    Spacer(modifier = Modifier.height(16.dp))

                    // X-axis rotation slider
                    Text("X Rotation: ${String.format("%.1f", rotationOffsetX * 180f / PI.toFloat())}°")
                    Slider(
                        value = rotationOffsetX,
                        onValueChange = { value ->
                            rotationOffsetX = value
                            engine.onRotate(
                                initialRotationX + rotationOffsetX,
                                initialRotationY + rotationOffsetY,
                                initialRotationZ + rotationOffsetZ
                            )
                            engine.draw()
                            viewModel.updateRoomModelRotation(rotationOffsetX, rotationOffsetY, rotationOffsetZ)
                        },
                        valueRange = -PI.toFloat()..PI.toFloat(),
                        modifier = Modifier.fillMaxWidth()
                    )

                    Spacer(modifier = Modifier.height(8.dp))

                    // Y-axis rotation slider
                    Text("Y Rotation: ${String.format("%.1f", rotationOffsetY * 180f / PI.toFloat())}°")
                    Slider(
                        value = rotationOffsetY,
                        onValueChange = { value ->
                            rotationOffsetY = value
                            engine.onRotate(
                                initialRotationX + rotationOffsetX,
                                initialRotationY + rotationOffsetY,
                                initialRotationZ + rotationOffsetZ
                            )
                            engine.draw()
                            viewModel.updateRoomModelRotation(rotationOffsetX, rotationOffsetY, rotationOffsetZ)
                        },
                        valueRange = -PI.toFloat()..PI.toFloat(),
                        modifier = Modifier.fillMaxWidth()
                    )

                    Spacer(modifier = Modifier.height(8.dp))

                    // Z-axis rotation slider
                    Text("Z Rotation: ${String.format("%.1f", rotationOffsetZ * 180f / PI.toFloat())}°")
                    Slider(
                        value = rotationOffsetZ,
                        onValueChange = { value ->
                            rotationOffsetZ = value
                            engine.onRotate(
                                initialRotationX + rotationOffsetX,
                                initialRotationY + rotationOffsetY,
                                initialRotationZ + rotationOffsetZ
                            )
                            engine.draw()
                            viewModel.updateRoomModelRotation(rotationOffsetX, rotationOffsetY, rotationOffsetZ)
                        },
                        valueRange = -PI.toFloat()..PI.toFloat(),
                        modifier = Modifier.fillMaxWidth()
                    )

                    Spacer(modifier = Modifier.height(16.dp))

                    // Reset button
                    Button(
                        onClick = { resetRotation() },
                        modifier = Modifier.fillMaxWidth()
                    ) {
                        Text("Reset to Default")
                    }
                }
            }
        }
    }
}
