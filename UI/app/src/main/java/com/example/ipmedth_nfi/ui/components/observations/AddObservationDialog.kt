package com.example.ipmedth_nfi.ui.components.observations

import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.material3.TextField
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp

@Composable
fun AddObservationDialog(
    onConfirm: (String, String, String) -> Unit,
    onDismiss: () -> Unit
) {
    var beschrijving by remember { mutableStateOf("") }
    var locatie by remember { mutableStateOf("") }
    var notities by remember { mutableStateOf("") }

    AlertDialog(
        onDismissRequest = onDismiss,
        title = {
            Text(text = "Nieuwe observatie")
        },
        text = {
            Column {
                TextField(
                    value = beschrijving,
                    onValueChange = { beschrijving = it },
                    label = { Text("Beschrijving") },
                    modifier = Modifier.fillMaxWidth()
                )

                Spacer(Modifier.height(8.dp))

                TextField(
                    value = locatie,
                    onValueChange = { locatie = it },
                    label = { Text("Locatie") },
                    modifier = Modifier.fillMaxWidth()
                )

                Spacer(Modifier.height(8.dp))

                TextField(
                    value = notities,
                    onValueChange = { notities = it },
                    label = { Text("Notities") },
                    modifier = Modifier.fillMaxWidth(),
                    minLines = 4
                )
            }
        },
        confirmButton = {
            TextButton(
                onClick = {
                    onConfirm(beschrijving, locatie, notities)
                }
            ) {
                Text("Toevoegen")
            }
        },
        dismissButton = {
            TextButton(onClick = onDismiss) {
                Text("Annuleren")
            }
        }
    )
}