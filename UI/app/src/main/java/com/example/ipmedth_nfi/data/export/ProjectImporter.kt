package com.example.ipmedth_nfi.data.export

import android.content.Context
import android.net.Uri
import com.example.ipmedth_nfi.data.persistence.ProjectSnapshot
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import kotlinx.serialization.json.Json
import java.io.File
import java.io.FileOutputStream
import java.util.UUID
import java.util.zip.ZipInputStream

object ProjectImporter {

    // Import a ZIP from a content Uri, extract and return the contained ProjectSnapshot (or null on failure).
    suspend fun importProjectFromUri(context: Context, uri: Uri): ProjectSnapshot? = withContext(Dispatchers.IO) {
        try {
            val tmpDir = File(context.cacheDir, "import_tmp_${System.currentTimeMillis()}")
            if (tmpDir.exists()) tmpDir.deleteRecursively()
            tmpDir.mkdirs()

            // Copy zip from content uri to a temp file
            val tmpZip = File(tmpDir, "import.zip")
            context.contentResolver.openInputStream(uri)?.use { input ->
                FileOutputStream(tmpZip).use { out ->
                    input.copyTo(out)
                }
            } ?: return@withContext null

            // Extract zip
            ZipInputStream(tmpZip.inputStream()).use { zis ->
                var entry = zis.nextEntry
                while (entry != null) {
                    val outFile = File(tmpDir, entry.name)
                    if (entry.isDirectory) {
                        outFile.mkdirs()
                    } else {
                        outFile.parentFile?.mkdirs()
                        FileOutputStream(outFile).use { fos ->
                            zis.copyTo(fos)
                        }
                    }
                    zis.closeEntry()
                    entry = zis.nextEntry
                }
            }

            // read project_state.json
            val jsonFile = File(tmpDir, "project_state.json")
            if (!jsonFile.exists()) {
                tmpDir.deleteRecursively()
                return@withContext null
            }
            val jsonText = jsonFile.readText()
            val json = Json { ignoreUnknownKeys = true }
            val snapshot = json.decodeFromString(ProjectSnapshot.serializer(), jsonText)

            // Generate new internal ID for imported project
            val newInternalId = UUID.randomUUID().toString()

            // Copy custom model to the new project directory if it exists
            var newCustomModelPath: String? = snapshot.roomModel?.customModelPath
            if (snapshot.roomModel?.customModelPath != null) {
                val oldModelFile = File(snapshot.roomModel.customModelPath)
                val modelsInZip = File(tmpDir, "models")
                if (modelsInZip.exists()) {
                    // Find the model file in the extracted ZIP
                    val modelFile = modelsInZip.listFiles()?.find { it.name == oldModelFile.name }
                    if (modelFile != null && modelFile.exists()) {
                        // Create models directory in the new project location
                        val storageManager = ProjectStorageManager(context)
                        val newOnderzoek = snapshot.onderzoek.copy(internalId = newInternalId)
                        val projectDir = storageManager.getProjectDir(newOnderzoek)
                        val newModelsDir = File(projectDir, "models")
                        newModelsDir.mkdirs()

                        // Copy the model file to the new location
                        val newModelFile = File(newModelsDir, modelFile.name)
                        modelFile.copyTo(newModelFile, overwrite = true)
                        newCustomModelPath = newModelFile.absolutePath
                    }
                }
            }

            val fixedSnapshot = snapshot.copy(
                onderzoek = snapshot.onderzoek.copy(
                    internalId = newInternalId
                ),
                roomModel = snapshot.roomModel?.copy(
                    customModelPath = newCustomModelPath
                )
            )

            tmpDir.deleteRecursively()
            return@withContext fixedSnapshot
        } catch (t: Throwable) {
            t.printStackTrace()
            return@withContext null
        }
    }
}