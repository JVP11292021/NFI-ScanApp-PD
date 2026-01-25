package com.example.ipmedth_nfi.ui.components.thema

import androidx.compose.material3.AlertDialog
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable

@Composable
fun AddHoofdthemaChooserDialog(
    onManual: () -> Unit,
    onDelict: () -> Unit,
    onDismiss: () -> Unit
) {
    AlertDialog(
        onDismissRequest = onDismiss,
        title = {
            Text("Hoofdthema toevoegen")
        },
        text = {
            Text("Hoe wil je hoofdthema’s toevoegen?")
        },
        confirmButton = {
            TextButton(onClick = onManual) {
                Text("Handmatig")
            }
        },
        dismissButton = {
            TextButton(onClick = onDelict) {
                Text("Op basis van delict")
            }
        }
    )
}
