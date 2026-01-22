package com.example.ipmedth_nfi.viewmodel

import java.io.File
import java.util.UUID
import android.app.Application
import android.net.Uri
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateListOf
import androidx.compose.runtime.mutableStateMapOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.lifecycle.AndroidViewModel
import androidx.lifecycle.viewModelScope
import com.example.ipmedth_nfi.data.persistence.ProjectSnapshot
import com.example.ipmedth_nfi.model.ExportData
import com.example.ipmedth_nfi.model.Hoofdthema
import com.example.ipmedth_nfi.model.Marker
import com.example.ipmedth_nfi.model.Observation
import com.example.ipmedth_nfi.model.Onderzoek
import com.example.ipmedth_nfi.model.ProjectStorage
import com.example.ipmedth_nfi.model.RoomModel
import com.example.ipmedth_nfi.ui.navigation.AssessmentPage
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.SharingStarted
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.map
import kotlinx.coroutines.flow.stateIn
import java.time.Instant

class SessionViewModel(
    application: Application,
    private val storage: ProjectStorage
) : AndroidViewModel(application) {
    private val _activeOnderzoek = MutableStateFlow<Onderzoek?>(null)
    val activeOnderzoek: StateFlow<Onderzoek?> = _activeOnderzoek

    val hasActiveOnderzoek: StateFlow<Boolean> =
        _activeOnderzoek.map { it != null }
            .stateIn(
                scope = viewModelScope,
                started = SharingStarted.Eagerly,
                initialValue = false
            )

    fun startOnderzoek(onderzoek: Onderzoek) {
        _activeOnderzoek.value = onderzoek

        storage.loadSnapshot(onderzoek)?.let { snapshot ->
            infoVooraf.clear()
            infoVooraf.addAll(snapshot.infoVooraf)

            infoTerPlaatse.clear()
            infoTerPlaatse.addAll(snapshot.infoTerPlaatse)

            observations.clear()
            observations.addAll(snapshot.observations)

            hoofdthemas.clear()
            hoofdthemas.addAll(snapshot.hoofdthemas)

            markers.clear()
            markers.addAll(snapshot.markers)

            appData.clear()
            appData.putAll(snapshot.appData)

            roomModel = snapshot.roomModel
        }
    }


    fun closeOnderzoek() {
        _activeOnderzoek.value = null
    }
    
    val pageCompletion = mutableStateMapOf<AssessmentPage, Boolean>().apply {
        AssessmentPage.all.forEach { page ->
            this[page] = false
        }
    }

    val markers = mutableStateListOf<Marker>()
    val appData = mutableStateMapOf<String, String>()
    var roomModel: RoomModel? by mutableStateOf(null)
    var currentAssessmentPage by mutableStateOf<AssessmentPage>(AssessmentPage.Info)
        private set

    // Image handler functionality
    fun onPhotoCaptured(uri: Uri) {
        // TODO: Forward to ExportManager
        println("Captured photo: $uri")
    }

    fun createImageFile(): File {
        val onderzoek = _activeOnderzoek.value
            ?: error("No active onderzoek")

        val imageDir = storage.getImageDir(onderzoek)

        if (!imageDir.exists()) {
            imageDir.mkdirs()
        }

        val fileName = "IMG_${UUID.randomUUID()}.jpg"
        return File(imageDir, fileName)
    }

    // Info page functionality
    val infoVooraf = mutableStateListOf<String>()
    val infoTerPlaatse = mutableStateListOf<String>()

    fun addInfoVooraf(text: String) {
        infoVooraf.add(text)
        autoSave()
    }

    fun removeInfoVooraf(index: Int) {
        if (index in infoVooraf.indices) {
            infoVooraf.removeAt(index)
            autoSave()
        }
    }

    fun addInfoTerPlaatse(text: String) {
        infoTerPlaatse.add(text)
        autoSave()
    }

    fun removeInfoTerPlaatse(index: Int) {
        if (index in infoTerPlaatse.indices) {
            infoTerPlaatse.removeAt(index)
            autoSave()
        }
    }

    fun setAssessmentPage(page: AssessmentPage) {
        currentAssessmentPage = page
    }

    // Observation page functionality
    val observations = mutableStateListOf<Observation>()

    fun addObservation(
        beschrijving: String,
        locatie: String,
        notities: String
    ) {
        observations += Observation(
            createdAt = Instant.now(),
            beschrijving = beschrijving,
            locatie = locatie,
            notities = notities
        )
        autoSave()
    }

    fun deleteObservation(id: String) {
        observations.removeAll { it.id == id }
        autoSave()
    }

    // Theme page functionality
    val hoofdthemas = mutableStateListOf<Hoofdthema>()

    fun addHoofdthema(thema: Hoofdthema) {
        hoofdthemas.add(thema)
        autoSave()
    }

    fun deleteHoofdthema(id: String) {
        hoofdthemas.removeAll { it.id == id }
        autoSave()
    }

    fun updateHoofdthema(updatedThema: Hoofdthema) {
        val index = hoofdthemas.indexOfFirst { it.id == updatedThema.id }
        if (index != -1) {
            hoofdthemas[index] = updatedThema
            autoSave()
        }
    }

    fun relevantCount(): Int = hoofdthemas.count { it.relevant }

    // Completion checkbox functionality
    fun toggleBookmark(id: String) {
        val index = observations.indexOfFirst { it.id == id }
        if (index != -1) {
            observations[index] =
                observations[index].copy(
                    isBookmarked = !observations[index].isBookmarked
                )
        }
        autoSave()
    }

    fun togglePageCompletion(page: AssessmentPage, completed: Boolean) {
        pageCompletion[page] = completed
        autoSave()
    }

    fun canCompleteFinish(): Boolean {
        return pageCompletion
            .filterKeys { it != AssessmentPage.Finish }
            .values
            .all { it }
    }

    // Export functionality
    fun buildExport(): ExportData {
        return ExportData(
            roomModel = roomModel,
            markers = markers.toList(),
            appData = appData.toMap()
        )
    }

    private fun buildSnapshot(): ProjectSnapshot {
        val onderzoek = _activeOnderzoek.value
            ?: error("No active onderzoek")

        return ProjectSnapshot(
            onderzoek = onderzoek,
            infoVooraf = infoVooraf.toList(),
            infoTerPlaatse = infoTerPlaatse.toList(),
            observations = observations.toList(),
            hoofdthemas = hoofdthemas.toList(),
            markers = markers.toList(),
            appData = appData.toMap(),
            roomModel = roomModel
        )
    }

    private fun autoSave() {
        val onderzoek = _activeOnderzoek.value
        if (onderzoek != null) {
            try {
                storage.saveSnapshot(buildSnapshot())
            } catch (e: Exception) {
                e.printStackTrace()
            }
        }
    }
}
