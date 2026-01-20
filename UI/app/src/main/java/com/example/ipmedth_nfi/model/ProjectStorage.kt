package com.example.ipmedth_nfi.model

import java.io.File

interface ProjectStorage {
    fun createProject(onderzoek: Onderzoek): File
    fun loadAllProjects(): List<Onderzoek>
    fun getProjectDir(onderzoek: Onderzoek): File
    fun getImageDir(onderzoek: Onderzoek): File
    fun deleteProject(onderzoek: Onderzoek): Boolean
}