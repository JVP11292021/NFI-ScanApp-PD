package com.example.ipmedth_nfi.ui.components.thema

import androidx.compose.foundation.layout.Column
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import com.example.ipmedth_nfi.model.DelictType
import com.example.ipmedth_nfi.model.Hoofdthema

@Composable
fun AddHoofdthemaFromDelictDialog(
    onConfirm: (List<Hoofdthema>) -> Unit,
    onDismiss: () -> Unit
) {
    AlertDialog(
        onDismissRequest = onDismiss,
        title = {
            Text("Kies type delict")
        },
        text = {
            Column {
                DelictType.entries.forEach { delict ->
                    TextButton(
                        onClick = {
                            onConfirm(
                                delict.hoofdthemas.map {
                                    Hoofdthema(
                                        name = it.displayName,
                                        relevant = true,
                                        onderbouwing = ""
                                    )
                                }
                            )
                        }
                    ) {
                        Text(delict.displayName)
                    }
                }
            }
        },
        confirmButton = {},
        dismissButton = {
            TextButton(onClick = onDismiss) {
                Text("Annuleren")
            }
        }
    )
}
