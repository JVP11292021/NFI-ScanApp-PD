package com.example.ipmedth_nfi.model

import com.example.ipmedth_nfi.data.persistence.ProjectSnapshot
import java.io.File

interface ProjectStorage {
    fun createProject(onderzoek: Onderzoek): File
    fun deleteProject(onderzoek: Onderzoek): Boolean
    fun loadAllProjects(): List<Onderzoek>
    fun getProjectDir(onderzoek: Onderzoek): File
    fun getImageDir(onderzoek: Onderzoek): File
    fun saveSnapshot(snapshot: ProjectSnapshot)
    fun loadSnapshot(onderzoek: Onderzoek): ProjectSnapshot?
}