package com.example.ipmedth_nfi.pages.model

import androidx.compose.foundation.gestures.detectTapGestures
import androidx.compose.foundation.gestures.detectTransformGestures
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.Button
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import com.example.ipmedth_nfi.bridge.NativeAndroidEngine
import com.example.ipmedth_nfi.ui.vk.VulkanRenderer
import com.example.ipmedth_nfi.viewmodel.SessionViewModel
import kotlin.math.abs
import kotlin.math.hypot

@Composable
fun AnnotationPage(
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

    // Get saved rotation from viewModel
    val savedRotation = viewModel.roomModel
    val rotationOffsetX = savedRotation?.rotationOffsetX ?: 0f
    val rotationOffsetY = savedRotation?.rotationOffsetY ?: 0f
    val rotationOffsetZ = savedRotation?.rotationOffsetZ ?: 0f

    // Find the action item from all aandachtspunten
    val selectedAction = remember(selectedMarkerActionId, viewModel.aandachtspunten) {
        viewModel.aandachtspunten
            .flatMap { it.primaryActions + it.otherActions }
            .find { it.id == selectedMarkerActionId }
    }

    if (showMarkerDialog && selectedAction != null) {
        AlertDialog(
            onDismissRequest = { showMarkerDialog = false },
            title = { Text("Marker Informatie") },
            text = {
                Column {
                    Text(
                        text = "Beschrijving en Locatie:",
                        style = MaterialTheme.typography.labelMedium,
                        fontWeight = FontWeight.Bold
                    )
                    Text(text = selectedAction.beschrijvingEnLocatie)

                    Spacer(modifier = Modifier.height(8.dp))

                    Text(
                        text = "Type:",
                        style = MaterialTheme.typography.labelMedium,
                        fontWeight = FontWeight.Bold
                    )
                    Text(text = selectedAction.type.name)

                    selectedAction.subType?.let { subType ->
                        Spacer(modifier = Modifier.height(8.dp))
                        Text(
                            text = "Subtype:",
                            style = MaterialTheme.typography.labelMedium,
                            fontWeight = FontWeight.Bold
                        )
                        Text(text = subType.name)
                    }

                    selectedAction.andersBeschrijving?.let { anders ->
                        Spacer(modifier = Modifier.height(8.dp))
                        Text(
                            text = "Anders:",
                            style = MaterialTheme.typography.labelMedium,
                            fontWeight = FontWeight.Bold
                        )
                        Text(text = anders)
                    }
                }
            },
            confirmButton = {
                Button(onClick = { showMarkerDialog = false }) {
                    Text("Sluiten")
                }
            }
        )
    }

    VulkanRenderer(
        engine = engine,
        projectDirPath = projectDirPath,
        actionId = actionId,
        onEngineReady = {
            // Apply saved rotation immediately after engine is initialized
            engine.setInitialRotation(rotationOffsetX, rotationOffsetY, rotationOffsetZ)
            engine.draw()
        },
        modifier = modifier
            .pointerInput(Unit) {
                detectTapGestures(
                    onTap = { offset ->
                        engine.onTap(offset.x, offset.y)
                        engine.draw()
                        // Check if a marker was tapped and get its action ID
                        val tappedActionId = engine.getLastTappedMarkerActionId()
                        if (tappedActionId.isNotEmpty()) {
                            selectedMarkerActionId = tappedActionId
                            showMarkerDialog = true
                        }
                    },
                    onDoubleTap = { offset ->
                        engine.onDoubleTap(offset.x, offset.y)
                        engine.draw()           // 1st draw to place/remove marker
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

