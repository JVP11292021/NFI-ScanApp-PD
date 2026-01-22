package com.example.ipmedth_nfi.ui

import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import com.example.ipmedth_nfi.model.Onderzoek


@Composable
fun DeleteProjectDialog(
    onderzoek: Onderzoek,
    onConfirm: () -> Unit,
    onDismiss: () -> Unit
) {
    var input by remember { mutableStateOf("") }
    val canDelete = input == onderzoek.onderzoeksnaam

    AlertDialog(
        onDismissRequest = onDismiss,
        title = { Text("Onderzoek verwijderen") },
        text = {
            Column {
                Text(
                    "Typ de onderzoeksnaam om te bevestigen:\n" +
                            "“${onderzoek.onderzoeksnaam}”"
                )
                Spacer(Modifier.height(12.dp))
                OutlinedTextField(
                    value = input,
                    onValueChange = { input = it },
                    modifier = Modifier.fillMaxWidth()
                )
            }
        },
        confirmButton = {
            Button(
                onClick = onConfirm,
                enabled = canDelete,
                colors = ButtonDefaults.buttonColors(
                    containerColor = MaterialTheme.colorScheme.error
                )
            ) {
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
