package com.example.ipmedth_nfi.data.export

import android.content.Context
import com.example.ipmedth_nfi.data.persistence.ProjectSnapshot
import kotlinx.serialization.json.Json
import java.io.BufferedInputStream
import java.io.BufferedOutputStream
import java.io.File
import java.io.FileInputStream
import java.io.FileOutputStream
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale
import java.util.zip.ZipEntry
import java.util.zip.ZipOutputStream
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

object ProjectExporter {

    private fun exportsRoot(context: Context): File {
        val dir = File(context.getExternalFilesDir(null), "exports")
        if (!dir.exists()) dir.mkdirs()
        return dir
    }

    suspend fun exportProjectZip(context: Context, snapshot: ProjectSnapshot): File = withContext(Dispatchers.IO) {
        val timestamp = SimpleDateFormat("yyyyMMdd_HHmmss", Locale.US).format(Date())
        val zipFile = File(exportsRoot(context), "project_${snapshot.onderzoek.zaaknummer}_$timestamp.zip")

        val json = Json { prettyPrint = true }
        val tmpDir = File(exportsRoot(context), "tmp_${timestamp}")
        if (tmpDir.exists()) tmpDir.deleteRecursively()
        tmpDir.mkdirs()

        // write snapshot JSON
        val jsonFile = File(tmpDir, "project_state.json")
        jsonFile.writeText(json.encodeToString(snapshot))

        // copy images (if any) using ProjectStorageManager
        val storageManager = ProjectStorageManager(context)
        val imageDir = storageManager.getImageDir(snapshot.onderzoek)
        if (imageDir.exists()) {
            val imagesOut = File(tmpDir, "images")
            imagesOut.mkdirs()
            imageDir.listFiles()?.forEach { f ->
                if (f.isFile) f.copyTo(File(imagesOut, f.name), overwrite = true)
            }
        }

        // zip tmpDir
        ZipOutputStream(BufferedOutputStream(FileOutputStream(zipFile))).use { zos ->
            fun addFileToZip(file: File, basePath: String) {
                val entryName = if (basePath.isEmpty()) file.name else "${basePath}/${file.name}"
                if (file.isDirectory) {
                    file.listFiles()?.forEach { child -> addFileToZip(child, entryName) }
                } else {
                    FileInputStream(file).use { fis ->
                        BufferedInputStream(fis).use { bis ->
                            val entry = ZipEntry(entryName)
                            zos.putNextEntry(entry)
                            bis.copyTo(zos)
                            zos.closeEntry()
                        }
                    }
                }
            }

            tmpDir.listFiles()?.forEach { f -> addFileToZip(f, "") }
        }

        tmpDir.deleteRecursively()
        return@withContext zipFile
    }
}
