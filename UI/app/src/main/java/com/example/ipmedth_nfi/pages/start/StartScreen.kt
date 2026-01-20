package com.example.ipmedth_nfi.pages.start

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Delete
import androidx.compose.material3.Button
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
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
import com.example.ipmedth_nfi.ui.DeleteProjectDialog
import com.example.ipmedth_nfi.viewmodel.AppViewModel

@Composable
fun StartScreen(
    appViewModel: AppViewModel,
    onProjectStarted: (Onderzoek) -> Unit
) {
    var zaaknummer by remember { mutableStateOf("") }
    var onderzoeksnaam by remember { mutableStateOf("") }
    var deleteTarget by remember { mutableStateOf<Onderzoek?>(null) }

    val canStart = zaaknummer.isNotBlank() && onderzoeksnaam.isNotBlank()
    val projecten by appViewModel.onderzoeken
    Column(Modifier.padding(8.dp)) {

        Column(Modifier.padding(24.dp)) {
            Text("Nieuw onderzoek", style = MaterialTheme.typography.headlineSmall)
        }

        OutlinedTextField(
            value = zaaknummer,
            onValueChange = { zaaknummer = it },
            label = { Text("Zaaknummer") },
            modifier = Modifier.fillMaxWidth()
        )

        OutlinedTextField(
            value = onderzoeksnaam,
            onValueChange = { onderzoeksnaam = it },
            label = { Text("Onderzoeksnaam") },
            modifier = Modifier.fillMaxWidth()
        )

        Button(
            onClick = {
                val onderzoek = Onderzoek(zaaknummer, onderzoeksnaam)
                appViewModel.startNewOnderzoek(
                    onderzoek
                )
                onProjectStarted(onderzoek)
            },
            enabled = canStart,
            modifier = Modifier.fillMaxWidth()
        ) {
            Text("Start nieuw onderzoek")
        }

        Spacer(Modifier.height(24.dp))

        Text("Mijn onderzoeken")

        projecten.forEach { onderzoek ->
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween
            ) {
                TextButton(onClick = {
                    appViewModel.loadOnderzoek(onderzoek)
                    onProjectStarted(onderzoek)
                }) {
                    Text("${onderzoek.zaaknummer} – ${onderzoek.onderzoeksnaam}")
                }

                IconButton(onClick = { deleteTarget = onderzoek }) {
                    Icon(Icons.Default.Delete, contentDescription = null)
                }
            }
        }

        deleteTarget?.let {
            DeleteProjectDialog(
                onderzoek = it,
                onConfirm = {
                    appViewModel.deleteOnderzoek(it)
                    deleteTarget = null
                },
                onDismiss = { deleteTarget = null }
            )
        }
    }
}