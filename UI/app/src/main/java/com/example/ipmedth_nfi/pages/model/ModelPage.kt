package com.example.ipmedth_nfi.pages.model

import androidx.compose.foundation.gestures.detectTapGestures
import androidx.compose.foundation.gestures.detectTransformGestures
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Settings
import androidx.compose.material.icons.filled.Close
import androidx.compose.material3.FloatingActionButton
import androidx.compose.material3.Icon
import androidx.compose.runtime.Composable
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
import com.example.ipmedth_nfi.ui.components.vk.MarkerInfoDialog
import com.example.ipmedth_nfi.ui.components.vk.RotationControlMenu
import com.example.ipmedth_nfi.ui.components.vk.RotationWarningDialog
import com.example.ipmedth_nfi.ui.components.vk.ViewOnlyBanner
import com.example.ipmedth_nfi.ui.vk.VulkanRenderer
import com.example.ipmedth_nfi.viewmodel.SessionViewModel
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
    var showMarkerDialog by remember { mutableStateOf(false) }
    var selectedMarkerActionId by remember { mutableStateOf("") }
    var selectedMarkerCoordinates by remember { mutableStateOf<FloatArray?>(null) }

    // Initial rotation values from AndroidEngine (in radians)
    val initialRotationX = Math.toRadians(9.0).toFloat()
    val initialRotationY = Math.toRadians(180.0).toFloat()
    val initialRotationZ = Math.toRadians(93.0).toFloat()

    // Rotation offset from initial values
    val savedRotation = viewModel.roomModel
    var rotationOffsetX by remember { mutableStateOf(savedRotation?.rotationOffsetX ?: 0f) }
    var rotationOffsetY by remember { mutableStateOf(savedRotation?.rotationOffsetY ?: 0f) }
    var rotationOffsetZ by remember { mutableStateOf(savedRotation?.rotationOffsetZ ?: 0f) }

    var showRotationMenu by remember { mutableStateOf(false) }
    var showConfirmationDialog by remember { mutableStateOf(false) }
    var markersCleared by remember { mutableStateOf(false) }

    val handleMenuOpen = {
        if (!markersCleared && engine.hasMarkers()) {
            showConfirmationDialog = true
        } else {
            showRotationMenu = true
        }
    }

    val resetRotation = {
        rotationOffsetX = 0f
        rotationOffsetY = 0f
        rotationOffsetZ = 0f
        engine.onRotate(initialRotationX, initialRotationY, initialRotationZ)
        engine.draw()
        viewModel.updateRoomModelRotation(0f, 0f, 0f)
    }

    val selectedAction = remember(selectedMarkerActionId, viewModel.aandachtspunten) {
        viewModel.aandachtspunten
            .flatMap { it.primaryActions + it.otherActions }
            .find { it.id == selectedMarkerActionId }
    }

    if (showMarkerDialog && selectedAction != null) {
        MarkerInfoDialog(
            action = selectedAction,
            onDismiss = { showMarkerDialog = false },
            coordinates = selectedMarkerCoordinates
        )
    }

    Column(modifier = modifier.fillMaxSize()) {
        ViewOnlyBanner()

        Box(modifier = Modifier.fillMaxSize()) {
            VulkanRenderer(
                engine = engine,
                projectDirPath = projectDirPath,
                actionId = actionId,
                onEngineReady = {
                    // Apply saved rotation immediately after engine is initialized
                engine.setInitialRotation(rotationOffsetX, rotationOffsetY, rotationOffsetZ)
                engine.draw()
            },
            modifier = Modifier
                .pointerInput(Unit) {
                    detectTapGestures(
                        onTap = { offset ->
                            engine.onTap(offset.x, offset.y)
                            engine.draw()
                            // Check if a marker was tapped and get its action ID and coordinates
                            val tappedActionId = engine.getLastTappedMarkerActionId()
                            if (tappedActionId.isNotEmpty()) {
                                selectedMarkerActionId = tappedActionId
                                selectedMarkerCoordinates = engine.getLastTappedMarkerPosition()
                                showMarkerDialog = true
                            }
                        },
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

        // Floating action button to open/close rotation menu
        FloatingActionButton(
            onClick = {
                if (showRotationMenu) {
                    showRotationMenu = false
                } else {
                    handleMenuOpen()
                }
            },
            modifier = Modifier
                .align(Alignment.BottomEnd)
                .padding(16.dp)
        ) {
            Icon(
                imageVector = if (showRotationMenu) Icons.Default.Close else Icons.Default.Settings,
                contentDescription = if (showRotationMenu) "Sluit rotatie instellingen" else "Open rotatie instellingen"
            )
        }

        // Rotation control menu
        RotationControlMenu(
            visible = showRotationMenu,
            rotationOffsetX = rotationOffsetX,
            rotationOffsetY = rotationOffsetY,
            rotationOffsetZ = rotationOffsetZ,
            onRotationXChange = { value ->
                rotationOffsetX = value
                engine.onRotate(
                    initialRotationX + rotationOffsetX,
                    initialRotationY + rotationOffsetY,
                    initialRotationZ + rotationOffsetZ
                )
                engine.draw()
                viewModel.updateRoomModelRotation(rotationOffsetX, rotationOffsetY, rotationOffsetZ)
            },
            onRotationYChange = { value ->
                rotationOffsetY = value
                engine.onRotate(
                    initialRotationX + rotationOffsetX,
                    initialRotationY + rotationOffsetY,
                    initialRotationZ + rotationOffsetZ
                )
                engine.draw()
                viewModel.updateRoomModelRotation(rotationOffsetX, rotationOffsetY, rotationOffsetZ)
            },
            onRotationZChange = { value ->
                rotationOffsetZ = value
                engine.onRotate(
                    initialRotationX + rotationOffsetX,
                    initialRotationY + rotationOffsetY,
                    initialRotationZ + rotationOffsetZ
                )
                engine.draw()
                viewModel.updateRoomModelRotation(rotationOffsetX, rotationOffsetY, rotationOffsetZ)
            },
            onReset = resetRotation,
            onClose = { showRotationMenu = false },
            modifier = Modifier
                .align(Alignment.BottomEnd)
                .padding(16.dp)
        )

        // Warning dialog
        RotationWarningDialog(
            visible = showConfirmationDialog,
            onConfirm = {
                engine.clearMarkers()
                engine.draw()
                markersCleared = true
                showConfirmationDialog = false
                showRotationMenu = true
            },
            onDismiss = {
                showConfirmationDialog = false
            }
        )
        }
    }
}
