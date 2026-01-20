package com.example.ipmedth_nfi.viewmodel

import androidx.compose.runtime.mutableStateOf
import androidx.lifecycle.ViewModel
import com.example.ipmedth_nfi.model.Onderzoek
import com.example.ipmedth_nfi.model.ProjectStorage
import androidx.compose.runtime.State


class AppViewModel(
    private val storage: ProjectStorage
) : ViewModel() {

    private val _onderzoeken = mutableStateOf<List<Onderzoek>>(emptyList())
    val onderzoeken: State<List<Onderzoek>> = _onderzoeken

    init {
        refreshOnderzoeken()
    }

    fun refreshOnderzoeken() {
        _onderzoeken.value = storage.loadAllProjects()
    }

    fun startNewOnderzoek(onderzoek: Onderzoek) {
        storage.createProject(onderzoek)
        refreshOnderzoeken()
    }

    fun deleteOnderzoek(onderzoek: Onderzoek) {
        if (storage.deleteProject(onderzoek)) {
            refreshOnderzoeken()
        }
    }

    fun loadOnderzoek(onderzoek: Onderzoek) {
        // optional: set active project here if needed
    }
}
