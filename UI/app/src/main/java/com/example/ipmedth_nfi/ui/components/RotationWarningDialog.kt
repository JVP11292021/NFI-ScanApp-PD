package com.example.ipmedth_nfi.ui.components

import androidx.compose.material3.AlertDialog
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable

@Composable
fun RotationWarningDialog(
    visible: Boolean,
    onConfirm: () -> Unit,
    onDismiss: () -> Unit
) {
    if (visible) {
        AlertDialog(
            onDismissRequest = onDismiss,
            title = { Text("Waarschuwing") },
            text = { Text("Het roteren van het model zal alle markers verwijderen. Weet u zeker dat u wilt doorgaan?") },
            confirmButton = {
                TextButton(onClick = onConfirm) {
                    Text("Bevestigen")
                }
            },
            dismissButton = {
                TextButton(onClick = onDismiss) {
                    Text("Annuleren")
                }
            }
        )
    }
}
