package com.example.ipmedth_nfi.viewmodel

import java.io.File
import java.util.UUID
import android.app.Application
import android.net.Uri
import android.os.Build
import androidx.annotation.RequiresApi
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateListOf
import androidx.compose.runtime.mutableStateMapOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.lifecycle.AndroidViewModel
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
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
        storage.createProject(onderzoek)
        _activeOnderzoek.value = onderzoek
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

    fun setAssessmentPage(page: AssessmentPage) {
        currentAssessmentPage = page
    }

    // Observation page functionality
    val observations = mutableStateListOf<Observation>()

    @RequiresApi(Build.VERSION_CODES.O)
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
    }

    fun deleteObservation(id: String) {
        observations.removeAll { it.id == id }
    }

    // Theme page functionality
    val hoofdthemas = mutableStateListOf<Hoofdthema>()

    fun addHoofdthema(thema: Hoofdthema) {
        hoofdthemas.add(thema)
    }

    fun deleteHoofdthema(id: String) {
        hoofdthemas.removeAll { it.id == id }
    }

    fun updateHoofdthema(updatedThema: Hoofdthema) {
        val index = hoofdthemas.indexOfFirst { it.id == updatedThema.id }
        if (index != -1) {
            hoofdthemas[index] = updatedThema
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
}
