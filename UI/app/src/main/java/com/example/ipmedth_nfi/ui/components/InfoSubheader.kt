package com.example.ipmedth_nfi.ui.components

import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.sp
import com.example.ipmedth_nfi.ui.components.buttons.SubheaderButton

@Composable
fun InfoSubheader(
    subject: String,
    details: String,
    actionTitle: String,
    onAddClick: () -> Unit
) {
    Row(
        modifier = Modifier.fillMaxWidth(),
        verticalAlignment = Alignment.CenterVertically
    ) {
        Column(
            modifier = Modifier.weight(1f)
        ) {
            Text(
                text = subject,
                color = MaterialTheme.colorScheme.primary,
                fontWeight = FontWeight.Bold
            )
            Text(
                text = details,
                color = Color.Gray,
                fontSize = 12.sp
            )
        }
        SubheaderButton(
            title = actionTitle,
            onClick = onAddClick,
            modifier = Modifier.weight(1f) // Fills the other half of the max width
        )
    }
}
