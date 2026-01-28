package com.example.ipmedth_nfi.ui.components.attention

import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.IntrinsicSize
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxHeight
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.outlined.ArrowRight
import androidx.compose.material.icons.outlined.Delete
import androidx.compose.material.icons.outlined.Details
import androidx.compose.material.icons.outlined.Edit
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.Card
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import com.example.ipmedth_nfi.model.Hoofdthema
@Composable
fun AttentionCard(
    title: String,
    theme: Hoofdthema,
    bulletPoints: List<String>,
    modifier: Modifier = Modifier,
    onAddBullet: (String) -> Unit,
    onUpdateBullets: (List<String>) -> Unit,
    onDelete: () -> Unit,
    onEdit: () -> Unit,
    onDetails: () -> Unit
) {
    var annotation by remember { mutableStateOf("") }
    var showDeleteConfirm by remember { mutableStateOf(false) }
    var isEditing by remember { mutableStateOf(false) }
    var editableBullets by remember(bulletPoints) {
        mutableStateOf(bulletPoints)
    }

    if (showDeleteConfirm) {
        AlertDialog(
            onDismissRequest = { showDeleteConfirm = false },
            title = { Text("Verwijderen?") },
            text = {
                Text("Weet je zeker dat je dit aandachtspunt wilt verwijderen?")
            },
            confirmButton = {
                IconButton(
                    onClick = {
                        showDeleteConfirm = false
                        onDelete()
                    }
                ) {
                    Text(
                        text = "Verwijder",
                        color = MaterialTheme.colorScheme.error,
                        fontWeight = FontWeight.Bold
                    )
                }
            },
            dismissButton = {
                IconButton(onClick = { showDeleteConfirm = false }) {
                    Text("Annuleren")
                }
            }
        )
    }

    Card(
        modifier = modifier
            .fillMaxWidth(),
//            .padding(horizontal = 16.dp),
        shape = RoundedCornerShape(16.dp)
    ) {
        Row(
            modifier = Modifier
                .padding(16.dp)
                .height(IntrinsicSize.Min)
        ) {
            Column(modifier = Modifier.weight(1f)) {
                Text(
                    text = title,
                    style = MaterialTheme.typography.titleMedium,
                    fontWeight = FontWeight.Bold
                )

                Spacer(Modifier.height(6.dp))

                ThemeMiniCard(theme = theme)

                Spacer(Modifier.height(12.dp))

                if (!isEditing) {
                    editableBullets.forEach {
                        Text("• $it", style = MaterialTheme.typography.bodyMedium)
                    }
                } else {
                    editableBullets.forEachIndexed { index, bullet ->
                        OutlinedTextField(
                            value = bullet,
                            onValueChange = {
                                editableBullets =
                                    editableBullets.toMutableList().also { list ->
                                        list[index] = it
                                    }
                            },
                            modifier = Modifier
                                .fillMaxWidth()
                                .padding(bottom = 6.dp),
                            singleLine = true
                        )
                    }
                }

                Spacer(Modifier.height(12.dp))

                OutlinedTextField(
                    value = annotation,
                    onValueChange = { annotation = it },
                    placeholder = { Text("Annotatie toevoegen…") },
                    modifier = Modifier.fillMaxWidth(),
                    singleLine = true,
                    trailingIcon = {
                        IconButton(
                            enabled = annotation.isNotBlank(),
                            onClick = {
                                onAddBullet(annotation.trim())
                                annotation = ""
                            }
                        ) {
                            Icon(
                                Icons.AutoMirrored.Outlined.ArrowRight,
                                contentDescription = "Toevoegen"
                            )
                        }
                    }
                )
            }

            Column(
                modifier = Modifier
                    .fillMaxHeight()
                    .padding(start = 8.dp),
                horizontalAlignment = Alignment.CenterHorizontally
            ) {
                IconButton(
                    onClick = {
                        if (isEditing) {
                            onUpdateBullets(editableBullets.filter { it.isNotBlank() })
                        }
                        isEditing = !isEditing
                    }
                ) {
                    Icon(
                        Icons.Outlined.Edit,
                        contentDescription = if (isEditing) "Opslaan" else "Bewerken"
                    )
                }

                Spacer(Modifier.weight(1f))

                IconButton(onClick = onDetails) {
                    Icon(Icons.Outlined.Details, contentDescription = "Details")
                }

                Spacer(Modifier.weight(1f))

                IconButton(onClick = { showDeleteConfirm = true }) {
                    Icon(
                        Icons.Outlined.Delete,
                        contentDescription = "Verwijderen",
                        tint = MaterialTheme.colorScheme.error
                    )
                }
            }
        }
    }
}
