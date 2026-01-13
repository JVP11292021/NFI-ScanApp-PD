package com.example.ipmedth_nfi.ui

import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp

@Composable
fun ObservationField(
    label: String,
    value: String,
    large: Boolean = false,
    color: Color = MaterialTheme.colorScheme.onSurface
) {
    Column(Modifier.padding(vertical = 4.dp)) {
        Text(
            text = value,
            color = color,
            style = if (large)
                MaterialTheme.typography.bodyMedium
            else
                MaterialTheme.typography.bodySmall
        )
    }
}