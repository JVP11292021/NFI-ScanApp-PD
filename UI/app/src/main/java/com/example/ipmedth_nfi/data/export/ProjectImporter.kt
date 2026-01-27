package com.example.ipmedth_nfi.data.export

import android.content.Context
import android.net.Uri
import com.example.ipmedth_nfi.data.persistence.ProjectSnapshot
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import kotlinx.serialization.json.Json
import java.io.File
import java.io.FileOutputStream
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

            tmpDir.deleteRecursively()
            return@withContext snapshot
        } catch (t: Throwable) {
            t.printStackTrace()
            return@withContext null
        }
    }
}