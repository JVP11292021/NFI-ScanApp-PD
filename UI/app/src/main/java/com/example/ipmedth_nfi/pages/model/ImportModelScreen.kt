package com.example.ipmedth_nfi.pages.model

import android.net.Uri
import android.widget.Toast
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Delete
import androidx.compose.material.icons.filled.Upload
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.unit.dp
import com.example.ipmedth_nfi.data.export.ProjectStorageManager
import com.example.ipmedth_nfi.viewmodel.SessionViewModel
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import java.io.File
import java.io.FileOutputStream

@Composable
fun ImportModelScreen(
    viewModel: SessionViewModel,
    onModelImported: () -> Unit = {}
) {
    val context = LocalContext.current
    val scope = rememberCoroutineScope()
    val activeOnderzoek = viewModel.activeOnderzoek.collectAsState().value

    var availableModels by remember { mutableStateOf<List<File>>(emptyList()) }
    var currentModelPath by remember { mutableStateOf<String?>(null) }
    var isLoading by remember { mutableStateOf(false) }

    // Load available models on first composition
    LaunchedEffect(activeOnderzoek) {
        activeOnderzoek?.let { onderzoek ->
            val storageManager = ProjectStorageManager(context)
            val projectDir = storageManager.getProjectDir(onderzoek)

            val modelsList = mutableListOf<File>()

            // Check models/ directory (imported models)
            val modelsDir = File(projectDir, "models")
            if (modelsDir.exists()) {
                modelsDir.listFiles()?.filter {
                    it.extension.lowercase() == "ply"
                }?.let { modelsList.addAll(it) }
            }

            // Check Reconstruction/sparse/ directory (reconstructed models)
            val sparseDir = File(projectDir, "Reconstruction/sparse")
            if (sparseDir.exists()) {
                sparseDir.listFiles()?.filter {
                    it.isFile && it.extension.lowercase() == "ply"
                }?.let {
                    modelsList.addAll(it)
                }
            }

            availableModels = modelsList

            // Get current model from snapshot
            currentModelPath = viewModel.roomModel?.customModelPath
        }
    }

    // File picker launcher for importing PLY files
    val launcher = rememberLauncherForActivityResult(ActivityResultContracts.OpenDocument()) { uri: Uri? ->
        if (uri != null && activeOnderzoek != null) {
            scope.launch {
                isLoading = true
                try {
                    withContext(Dispatchers.IO) {
                        val storageManager = ProjectStorageManager(context)
                        val projectDir = storageManager.getProjectDir(activeOnderzoek)
                        val modelsDir = File(projectDir, "models")
                        modelsDir.mkdirs()

                        // Copy the file
                        val fileName = uri.lastPathSegment?.substringAfterLast("/")
                            ?: "imported_${System.currentTimeMillis()}.ply"
                        val destFile = File(modelsDir, fileName)

                        context.contentResolver.openInputStream(uri)?.use { input ->
                            FileOutputStream(destFile).use { output ->
                                input.copyTo(output)
                            }
                        }

                        // Refresh list
                        withContext(Dispatchers.Main) {
                            val modelsList = mutableListOf<File>()

                            // Check models/ directory
                            val refreshModelsDir = File(projectDir, "models")
                            if (refreshModelsDir.exists()) {
                                refreshModelsDir.listFiles()?.filter {
                                    it.extension.lowercase() == "ply"
                                }?.let { modelsList.addAll(it) }
                            }

                            // Check Reconstruction/sparse/ directory
                            val sparseDir = File(projectDir, "Reconstruction/sparse")
                            if (sparseDir.exists()) {
                                sparseDir.listFiles()?.filter {
                                    it.isFile && it.extension.lowercase() == "ply"
                                }?.let { modelsList.addAll(it) }
                            }

                            availableModels = modelsList
                            Toast.makeText(
                                context,
                                "Model geïmporteerd: ${destFile.name}",
                                Toast.LENGTH_SHORT
                            ).show()
                        }
                    }
                } catch (e: Exception) {
                    withContext(Dispatchers.Main) {
                        Toast.makeText(
                            context,
                            "Import mislukt: ${e.message}",
                            Toast.LENGTH_LONG
                        ).show()
                    }
                } finally {
                    isLoading = false
                }
            }
        }
    }

    Box(Modifier.fillMaxSize(), contentAlignment = Alignment.TopCenter) {
        Column(
            Modifier
                .fillMaxWidth()
                .padding(16.dp),
            horizontalAlignment = Alignment.CenterHorizontally
        ) {
            Card(
                modifier = Modifier.fillMaxWidth(),
                colors = CardDefaults.cardColors(
                    containerColor = MaterialTheme.colorScheme.secondaryContainer
                )
            ) {
                Column(Modifier.padding(12.dp)) {
                    Text(
                        "Model Importeren",
                        style = MaterialTheme.typography.titleMedium
                    )
                    Spacer(Modifier.height(6.dp))
                    Text("Importeer een .ply bestand om als 3D model te gebruiken voor dit project.")
                }
            }

            Spacer(Modifier.height(16.dp))

            Button(
                modifier = Modifier.fillMaxWidth(),
                onClick = { launcher.launch(arrayOf("*/*")) },
                enabled = !isLoading
            ) {
                Icon(Icons.Default.Upload, contentDescription = null)
                Spacer(Modifier.width(8.dp))
                Text("Selecteer .ply bestand")
            }

            Spacer(Modifier.height(24.dp))

            if (isLoading) {
                CircularProgressIndicator()
                Spacer(Modifier.height(16.dp))
            }

            Text(
                "Beschikbare modellen",
                style = MaterialTheme.typography.titleMedium
            )

            Spacer(Modifier.height(8.dp))

            if (availableModels.isEmpty()) {
                Column {
                    Text(
                        "Geen modellen beschikbaar. Importeer een .ply bestand om te beginnen.",
                        style = MaterialTheme.typography.bodySmall,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )

                    // Debug info
                    activeOnderzoek?.let { onderzoek ->
                        val storageManager = ProjectStorageManager(context)
                        val projectDir = storageManager.getProjectDir(onderzoek)
                        val sparseDir = File(projectDir, "Reconstruction/sparse")

                        Spacer(Modifier.height(8.dp))
                        Text(
                            "Debug: Zoekt in ${sparseDir.absolutePath}",
                            style = MaterialTheme.typography.labelSmall,
                            color = MaterialTheme.colorScheme.outline
                        )
                        Text(
                            "Map bestaat: ${sparseDir.exists()}, Bestanden: ${sparseDir.listFiles()?.size ?: 0}",
                            style = MaterialTheme.typography.labelSmall,
                            color = MaterialTheme.colorScheme.outline
                        )
                        sparseDir.listFiles()?.forEach { file ->
                            Text(
                                "- ${file.name} (${file.extension})",
                                style = MaterialTheme.typography.labelSmall,
                                color = MaterialTheme.colorScheme.outline
                            )
                        }
                    }
                }
            } else {
                LazyColumn(
                    verticalArrangement = Arrangement.spacedBy(8.dp),
                    modifier = Modifier.fillMaxWidth()
                ) {
                    items(availableModels) { file ->
                        ModelListItem(
                            file = file,
                            isSelected = currentModelPath == file.absolutePath,
                            onSelect = {
                                scope.launch {
                                    viewModel.setCustomModelPath(file.absolutePath)
                                    currentModelPath = file.absolutePath
                                    Toast.makeText(
                                        context,
                                        "Model geselecteerd: ${file.name}",
                                        Toast.LENGTH_SHORT
                                    ).show()
                                    onModelImported()
                                }
                            },
                            onDelete = {
                                scope.launch {
                                    withContext(Dispatchers.IO) {
                                        file.delete()
                                    }
                                    availableModels = availableModels.filter { it != file }
                                    if (currentModelPath == file.absolutePath) {
                                        viewModel.setCustomModelPath(null)
                                        currentModelPath = null
                                    }
                                    Toast.makeText(
                                        context,
                                        "Model verwijderd: ${file.name}",
                                        Toast.LENGTH_SHORT
                                    ).show()
                                }
                            }
                        )
                    }
                }
            }

            Spacer(Modifier.height(16.dp))

            OutlinedButton(
                modifier = Modifier.fillMaxWidth(),
                onClick = {
                    viewModel.setCustomModelPath("ASSET:simple_scene.ply")
                    currentModelPath = "ASSET:simple_scene.ply"
                    Toast.makeText(
                        context,
                        "Standaard model (simple_scene.ply) wordt gebruikt",
                        Toast.LENGTH_SHORT
                    ).show()
                },
                enabled = currentModelPath != "ASSET:simple_scene.ply"
            ) {
                Text("Gebruik standaard model")
            }
        }
    }
}

@Composable
private fun ModelListItem(
    file: File,
    isSelected: Boolean,
    onSelect: () -> Unit,
    onDelete: () -> Unit
) {
    Card(
        modifier = Modifier.fillMaxWidth(),
        colors = if (isSelected) {
            CardDefaults.cardColors(containerColor = MaterialTheme.colorScheme.primaryContainer)
        } else {
            CardDefaults.cardColors()
        }
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(12.dp),
            horizontalArrangement = Arrangement.SpaceBetween,
            verticalAlignment = Alignment.CenterVertically
        ) {
            Column(modifier = Modifier.weight(1f)) {
                Text(
                    file.name,
                    style = MaterialTheme.typography.bodyLarge
                )
                Text(
                    "${file.length() / 1024} KB",
                    style = MaterialTheme.typography.bodySmall,
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )
                // Show source directory
                val sourceLabel = when {
                    file.absolutePath.contains("Reconstruction/sparse") -> "Gereconstrueerd model"
                    file.absolutePath.contains("models") -> "Geïmporteerd model"
                    else -> ""
                }
                if (sourceLabel.isNotEmpty()) {
                    Text(
                        sourceLabel,
                        style = MaterialTheme.typography.labelSmall,
                        color = MaterialTheme.colorScheme.secondary
                    )
                }
            }

            Row {
                if (!isSelected) {
                    Button(onClick = onSelect) {
                        Text("Selecteer")
                    }
                } else {
                    Text(
                        "Geselecteerd",
                        style = MaterialTheme.typography.labelMedium,
                        color = MaterialTheme.colorScheme.primary
                    )
                }

                Spacer(Modifier.width(8.dp))

                IconButton(onClick = onDelete) {
                    Icon(Icons.Default.Delete, contentDescription = "Verwijder")
                }
            }
        }
    }
}
