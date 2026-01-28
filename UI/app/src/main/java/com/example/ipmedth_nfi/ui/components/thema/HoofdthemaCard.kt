package com.example.ipmedth_nfi.ui.components.thema

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Check
import androidx.compose.material.icons.filled.Delete
import androidx.compose.material.icons.filled.Edit
import androidx.compose.material3.Card
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedTextField
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
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import com.example.ipmedth_nfi.model.Hoofdthema
import com.example.ipmedth_nfi.ui.components.ExpandableSection

@Composable
fun HoofdthemaCard(
    thema: Hoofdthema,
    onUpdate: (Hoofdthema) -> Unit,
    onDelete: () -> Unit
) {
    var editing by remember { mutableStateOf(false) }
    var showDelete by remember { mutableStateOf(false) }

    var relevant by remember(thema.id) {
        mutableStateOf(thema.relevant)
    }

    var onderbouwing by remember(thema.id) {
        mutableStateOf(thema.onderbouwing)
    }

    var mogelijkheden by remember(thema.id) {
        mutableStateOf(thema.mogelijkheden)
    }

    Card(
        modifier = Modifier
            .fillMaxWidth(),
//            .padding(horizontal = 16.dp, vertical = 8.dp),
        shape = RoundedCornerShape(16.dp),
        elevation = CardDefaults.cardElevation(defaultElevation = 3.dp)
    ) {

        Row(
            modifier = Modifier
                .fillMaxWidth()
                .background(MaterialTheme.colorScheme.primary)
                .padding(horizontal = 12.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {

            IconButton(
                onClick = {
                    if (editing) {
                        onUpdate(
                            thema.copy(
                                onderbouwing = onderbouwing,
                                relevant = relevant,
                                mogelijkheden = mogelijkheden
                            )
                        )
                    }
                    editing = !editing
                }
            ) {
                Icon(
                    imageVector = if (editing) Icons.Default.Check else Icons.Default.Edit,
                    contentDescription = null,
                    tint = Color.White
                )
            }

            Text(
                text = thema.name,
                modifier = Modifier.weight(1f),
                textAlign = TextAlign.Center,
                style = MaterialTheme.typography.titleMedium,
                color = Color.White
            )

            IconButton(onClick = { showDelete = true }) {
                Icon(Icons.Default.Delete, null, tint = Color.White)
            }
        }

        if (editing) {
            SingleChoiceSegmentedButtonRow(
                modifier = Modifier.fillMaxWidth().padding(horizontal = 12.dp)
            ) {
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
        }

        Column(modifier = Modifier.padding(12.dp)) {

            ExpandableSection(title = "Onderbouwing") {
                if (editing) {
                    OutlinedTextField(
                        value = onderbouwing,
                        onValueChange = { onderbouwing = it },
                        modifier = Modifier.fillMaxWidth()
                    )
                } else {
                    Text(onderbouwing)
                }
            }

            Spacer(Modifier.height(8.dp))

            ExpandableSection("Mogelijkheden") {
                if (!editing) {
                    if (mogelijkheden.isEmpty()) {
                        Text(
                            "Geen mogelijkheden toegevoegd.",
                            color = MaterialTheme.colorScheme.onSurfaceVariant
                        )
                    } else {
                        mogelijkheden.forEach {
                            Text("• $it")
                        }
                    }
                } else {
                    var newItem by remember { mutableStateOf("") }

                    Column {
                        mogelijkheden.forEachIndexed { index, item ->
                            Row(
                                verticalAlignment = Alignment.CenterVertically,
                                modifier = Modifier.fillMaxWidth()
                            ) {
                                Text(
                                    text = item,
                                    modifier = Modifier.weight(1f)
                                )
                                IconButton(
                                    onClick = {
                                        mogelijkheden = mogelijkheden.toMutableList().apply {
                                            removeAt(index)
                                        }
                                    }
                                ) {
                                    Icon(Icons.Default.Delete, null)
                                }
                            }
                        }

                        Spacer(Modifier.height(8.dp))

                        OutlinedTextField(
                            value = newItem,
                            onValueChange = { newItem = it },
                            label = { Text("Nieuwe mogelijkheid") },
                            modifier = Modifier.fillMaxWidth()
                        )

                        Spacer(Modifier.height(4.dp))

                        TextButton(
                            enabled = newItem.isNotBlank(),
                            onClick = {
                                mogelijkheden = mogelijkheden + newItem
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

    if (showDelete) {
        ConfirmDeleteDialog(
            title = "Thema verwijderen",
            message = "Weet je zeker dat je dit hoofdthema wilt verwijderen?",
            onConfirm = {
                showDelete = false
                onDelete()
            },
            onDismiss = { showDelete = false }
        )
    }
}
