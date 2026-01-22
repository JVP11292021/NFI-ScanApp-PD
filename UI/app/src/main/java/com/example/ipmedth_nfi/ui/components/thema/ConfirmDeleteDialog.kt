package com.example.ipmedth_nfi.ui.components.thema

import androidx.compose.material3.AlertDialog
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable

@Composable
fun ConfirmDeleteDialog(
    title: String,
    message: String,
    onConfirm: () -> Unit,
    onDismiss: () -> Unit
) {
    AlertDialog(
        onDismissRequest = onDismiss,
        title = { Text(title) },
        text = { Text(message) },
        confirmButton = {
            TextButton(onClick = onConfirm) {
                Text("Verwijderen")
            }
        },
        dismissButton = {
            TextButton(onClick = onDismiss) {
                Text("Annuleren")
            }
        }
    )
}
