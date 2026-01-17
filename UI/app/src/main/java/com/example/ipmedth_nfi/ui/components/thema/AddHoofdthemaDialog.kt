package com.example.ipmedth_nfi.ui.components.thema

import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.material3.TextField
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import com.example.ipmedth_nfi.model.Hoofdthema
import com.example.ipmedth_nfi.model.HoofdthemaType

@Composable
fun AddHoofdthemaDialog(
    onConfirm: (Hoofdthema) -> Unit,
    onDismiss: () -> Unit
) {
    var selectedType by remember { mutableStateOf<HoofdthemaType?>(null) }
    var onderbouwing by remember { mutableStateOf("") }

    AlertDialog(
        onDismissRequest = onDismiss,
        title = {
            Text("Nieuw hoofdthema")
        },
        text = {
            Column {

                HoofdthemaDropdown(
                    selected = selectedType,
                    onSelected = { selectedType = it }
                )

                Spacer(Modifier.height(8.dp))

                TextField(
                    value = onderbouwing,
                    onValueChange = { onderbouwing = it },
                    label = { Text("Onderbouwing") },
                    modifier = Modifier.fillMaxWidth(),
                    minLines = 3
                )
            }
        },
        confirmButton = {
            TextButton(
                enabled = selectedType != null,
                onClick = {
                    onConfirm(
                        Hoofdthema(
                            name = selectedType!!.displayName,
                            relevant = true,
                            onderbouwing = onderbouwing
                        )
                    )
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
