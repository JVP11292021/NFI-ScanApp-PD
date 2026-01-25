package com.example.ipmedth_nfi.ui.components.thema

import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.Checkbox
import androidx.compose.material3.SegmentedButton
import androidx.compose.material3.SegmentedButtonDefaults
import androidx.compose.material3.SingleChoiceSegmentedButtonRow
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import com.example.ipmedth_nfi.model.DelictType
import com.example.ipmedth_nfi.model.Hoofdthema

@Composable
fun DelictPresetDialog(
    delict: DelictType,
    onConfirm: (List<Hoofdthema>) -> Unit,
    onDismiss: () -> Unit
) {
    var selections by remember {
        mutableStateOf(
            delict.hoofdthemas.associate {
                it to true // selected
            }
        )
    }

    var relevant by remember { mutableStateOf(true) }

    AlertDialog(
        onDismissRequest = onDismiss,
        title = { Text(delict.displayName) },
        text = {
            Column {
                // Relevance toggle (applies to all)
                SingleChoiceSegmentedButtonRow {
                    SegmentedButton(
                        selected = relevant,
                        onClick = { relevant = true },
                        shape = SegmentedButtonDefaults.itemShape(0, 2)
                    ) { Text("Relevant") }

                    SegmentedButton(
                        selected = !relevant,
                        onClick = { relevant = false },
                        shape = SegmentedButtonDefaults.itemShape(1, 2)
                    ) { Text("Niet relevant") }
                }

                Spacer(Modifier.height(12.dp))

                selections.forEach { (type, selected) ->
                    Row(
                        Modifier
                            .fillMaxWidth()
                            .clickable {
                                selections = selections.toMutableMap().apply {
                                    put(type, !selected)
                                }
                            }
                            .padding(vertical = 8.dp),
                        verticalAlignment = Alignment.CenterVertically
                    ) {
                        Checkbox(
                            checked = selected,
                            onCheckedChange = {
                                selections = selections.toMutableMap().apply {
                                    put(type, it)
                                }
                            }
                        )
                        Text(type.displayName)
                    }
                }
            }
        },
        confirmButton = {
            TextButton(onClick = {
                onConfirm(
                    selections
                        .filterValues { it }
                        .keys
                        .map {
                            Hoofdthema(
                                name = it.displayName,
                                relevant = relevant,
                                onderbouwing = ""
                            )
                        }
                )
            }) {
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
