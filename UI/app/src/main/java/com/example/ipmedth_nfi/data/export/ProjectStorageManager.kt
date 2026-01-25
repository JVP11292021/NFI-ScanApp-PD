package com.example.ipmedth_nfi.data.export

import android.content.Context
import com.example.ipmedth_nfi.data.persistence.ProjectSnapshot
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
        File(getProjectDir(onderzoek), "Reconstruction/images")

    override fun createProject(onderzoek: Onderzoek): File {
        val projectDir = getProjectDir(onderzoek)
        val imageDir = getImageDir(onderzoek)

        imageDir.mkdirs()
        File(projectDir, "markers.txt").createNewFile()

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

    override fun saveSnapshot(snapshot: ProjectSnapshot) {
        val file = File(
            getProjectDir(snapshot.onderzoek),
            "project_state.json"
        )

        file.writeText(
            Json.encodeToString(snapshot)
        )
    }

    override fun loadSnapshot(onderzoek: Onderzoek): ProjectSnapshot? {
        val file = File(
            getProjectDir(onderzoek),
            "project_state.json"
        )

        if (!file.exists()) return null

        return runCatching {
            Json.decodeFromString<ProjectSnapshot>(file.readText())
        }.getOrNull()
    }


}
