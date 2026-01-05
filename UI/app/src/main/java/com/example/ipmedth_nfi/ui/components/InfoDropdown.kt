package com.example.ipmedth_nfi.ui.components

import androidx.compose.animation.AnimatedVisibility
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Edit
import androidx.compose.material.icons.filled.KeyboardArrowDown
import androidx.compose.material.icons.filled.KeyboardArrowUp
import androidx.compose.material3.Icon
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp

@Composable
fun InfoDropdown(
    title: String,
    modifier: Modifier = Modifier
) {
    // State to track if the dropdown is expanded or collapsed
    var isExpanded by remember { mutableStateOf(false) }

    Column(modifier = modifier.fillMaxWidth()) {
        // Header Row
        Box(
            modifier = Modifier
                .fillMaxWidth()
                .clickable { isExpanded = !isExpanded }
                .clip(RoundedCornerShape(16.dp))
                .background(Color.White)
                .padding(16.dp)

        ) {
            if (isExpanded) {
                Icon(
                    imageVector = Icons.Default.Edit,
                    contentDescription = "Edit entries",
                    modifier = Modifier
                        .align(Alignment.CenterStart)
                        .clickable { /* TODO: Implement edit logic */ }
                )
            }

            Text(
                text = title,
                modifier = Modifier.align(Alignment.Center)
                // To change color: color = androidx.compose.ui.graphics.Color.Black
                // To change font/size: style = androidx.compose.material3.MaterialTheme.typography.bodyLarge
            )

            Icon(
                imageVector = if (isExpanded) Icons.Default.KeyboardArrowUp else Icons.Default.KeyboardArrowDown,
                contentDescription = if (isExpanded) "Collapse" else "Expand",
                modifier = Modifier.align(Alignment.CenterEnd)
            )
        }

        // Dropdown Content
        AnimatedVisibility(visible = isExpanded) {
            Column(modifier = Modifier.padding(horizontal = 16.dp, vertical = 8.dp)) {
                // Template content
                Text(text = "Details for $title go here.")
                // You can add more Composables here
            }
        }
    }
}
