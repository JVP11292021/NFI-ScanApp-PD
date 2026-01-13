package com.example.ipmedth_nfi.viewmodel

import android.net.Uri
import android.os.Build
import androidx.annotation.RequiresApi
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateListOf
import androidx.compose.runtime.mutableStateMapOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.lifecycle.ViewModel
import com.example.ipmedth_nfi.model.ExportData
import com.example.ipmedth_nfi.model.Marker
import com.example.ipmedth_nfi.model.Observation
import com.example.ipmedth_nfi.model.RoomModel
import com.example.ipmedth_nfi.ui.navigation.AssessmentPage
import java.time.Instant

class SessionViewModel : ViewModel() {
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

    // New lists for InfoTab
    val infoVooraf = mutableStateListOf<String>()
    val infoTerPlaatse = mutableStateListOf<String>()

    fun setAssessmentPage(page: AssessmentPage) {
        currentAssessmentPage = page
    }

    fun buildExport(): ExportData {
        return ExportData(
            roomModel = roomModel,
            markers = markers.toList(),
            appData = appData.toMap()
        )
    }

    fun canCompleteFinish(): Boolean {
        return pageCompletion
            .filterKeys { it != AssessmentPage.Finish }
            .values
            .all { it }
    }

    fun onPhotoCaptured(uri: Uri) {
        // TODO: Forward to ExportManager
        println("Captured photo: $uri")
    }

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

    fun toggleBookmark(id: String) {
        val index = observations.indexOfFirst { it.id == id }
        if (index != -1) {
            observations[index] =
                observations[index].copy(
                    isBookmarked = !observations[index].isBookmarked
                )
        }
    }
}
