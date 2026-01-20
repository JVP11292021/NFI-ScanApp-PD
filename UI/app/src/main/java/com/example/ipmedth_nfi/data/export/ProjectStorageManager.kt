package com.example.ipmedth_nfi.data.export

import android.content.Context
import com.example.ipmedth_nfi.model.Onderzoek
import com.example.ipmedth_nfi.model.ProjectStorage
import kotlinx.serialization.json.Json
import java.io.File

class ProjectStorageManager(
    private val context: Context
) : ProjectStorage {

    private val rootDir =
        File(context.getExternalFilesDir(null), "NFI_Scanapp")

    override fun getProjectDir(onderzoek: Onderzoek): File =
        File(rootDir, "${onderzoek.zaaknummer}/${onderzoek.onderzoeksnaam}")

    override fun getImageDir(onderzoek: Onderzoek): File =
        File(getProjectDir(onderzoek), "reconstructie/images")

    override fun createProject(onderzoek: Onderzoek): File {
        val projectDir = getProjectDir(onderzoek)
        val imageDir = getImageDir(onderzoek)

        imageDir.mkdirs()

        File(projectDir, "project.json").writeText(
            Json.encodeToString(onderzoek)
        )

        return projectDir
    }

    override fun loadAllProjects(): List<Onderzoek> {
        if (!rootDir.exists()) return emptyList()

        return rootDir.walkTopDown()
            .filter { it.name == "project.json" }
            .mapNotNull {
                runCatching {
                    Json.decodeFromString<Onderzoek>(it.readText())
                }.getOrNull()
            }
            .toList()
    }

    override fun deleteProject(onderzoek: Onderzoek): Boolean {
        val dir = getProjectDir(onderzoek)
        return dir.exists() && dir.deleteRecursively()
    }
}
