package com.example.ipmedth_nfi.ui.components

import androidx.compose.animation.AnimatedVisibility
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Close
import androidx.compose.material3.Button
import androidx.compose.material3.Card
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Slider
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import kotlin.math.PI

@Composable
fun RotationControlMenu(
    visible: Boolean,
    rotationOffsetX: Float,
    rotationOffsetY: Float,
    rotationOffsetZ: Float,
    onRotationXChange: (Float) -> Unit,
    onRotationYChange: (Float) -> Unit,
    onRotationZChange: (Float) -> Unit,
    onReset: () -> Unit,
    onClose: () -> Unit,
    modifier: Modifier = Modifier
) {
    AnimatedVisibility(
        visible = visible,
        modifier = modifier
    ) {
        Card(
            modifier = Modifier.fillMaxWidth(0.85f)
        ) {
            Column(
                modifier = Modifier.padding(16.dp)
            ) {
                // Header
                Row(
                    verticalAlignment = Alignment.CenterVertically,
                    modifier = Modifier.fillMaxWidth()
                ) {
                    Text(
                        text = "Rotatie Instellingen",
                        style = MaterialTheme.typography.titleMedium,
                        modifier = Modifier.weight(1f)
                    )
                    IconButton(onClick = onClose) {
                        Icon(Icons.Default.Close, contentDescription = "Sluiten")
                    }
                }

                Spacer(modifier = Modifier.height(16.dp))

                // X-axis rotation slider
                Text("X Rotatie: ${String.format("%.1f", rotationOffsetX * 180f / PI.toFloat())}°")
                Slider(
                    value = rotationOffsetX,
                    onValueChange = onRotationXChange,
                    valueRange = -PI.toFloat()..PI.toFloat(),
                    modifier = Modifier.fillMaxWidth()
                )

                Spacer(modifier = Modifier.height(8.dp))

                // Y-axis rotation slider
                Text("Y Rotatie: ${String.format("%.1f", rotationOffsetY * 180f / PI.toFloat())}°")
                Slider(
                    value = rotationOffsetY,
                    onValueChange = onRotationYChange,
                    valueRange = -PI.toFloat()..PI.toFloat(),
                    modifier = Modifier.fillMaxWidth()
                )

                Spacer(modifier = Modifier.height(8.dp))

                // Z-axis rotation slider
                Text("Z Rotatie: ${String.format("%.1f", rotationOffsetZ * 180f / PI.toFloat())}°")
                Slider(
                    value = rotationOffsetZ,
                    onValueChange = onRotationZChange,
                    valueRange = -PI.toFloat()..PI.toFloat(),
                    modifier = Modifier.fillMaxWidth()
                )

                Spacer(modifier = Modifier.height(16.dp))

                // Reset button
                Button(
                    onClick = onReset,
                    modifier = Modifier.fillMaxWidth()
                ) {
                    Text("Herstel naar Standaard")
                }
            }
        }
    }
}
