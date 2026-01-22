package com.example.ipmedth_nfi.ui.components

import androidx.compose.animation.AnimatedVisibility
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Delete
import androidx.compose.material.icons.filled.KeyboardArrowUp
import androidx.compose.material3.Card
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.OutlinedTextField
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

@Composable
fun EditableListSection(
    title: String,
    items: List<String>,
    onChange: (List<String>) -> Unit
) {
    var expanded by remember { mutableStateOf(false) }
    var newItem by remember { mutableStateOf("") }

    Card {
        Column {
            Row(
                Modifier
                    .fillMaxWidth()
                    .clickable { expanded = !expanded }
                    .padding(16.dp),
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text(title, Modifier.weight(1f))
                Icon(
                    if (expanded) Icons.Default.KeyboardArrowUp
                    else Icons.Default.KeyboardArrowUp,
                    null
                )
            }

            AnimatedVisibility(expanded) {
                Column(Modifier.padding(16.dp)) {
                    items.forEachIndexed { index, item ->
                        Row(verticalAlignment = Alignment.CenterVertically) {
                            Text(item, Modifier.weight(1f))
                            IconButton(onClick = {
                                onChange(items.toMutableList().apply {
                                    removeAt(index)
                                })
                            }) {
                                Icon(Icons.Default.Delete, null)
                            }
                        }
                    }

                    OutlinedTextField(
                        value = newItem,
                        onValueChange = { newItem = it },
                        label = { Text("Nieuw") },
                        modifier = Modifier.fillMaxWidth()
                    )

                    TextButton(
                        enabled = newItem.isNotBlank(),
                        onClick = {
                            onChange(items + newItem)
                            newItem = ""
                        }
                    ) {
                        Text("Toevoegen")
                    }
                }
            }
        }
    }
}
