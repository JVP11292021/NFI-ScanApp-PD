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
import com.example.ipmedth_nfi.model.Aandachtspunt
import com.example.ipmedth_nfi.model.ExportData
import com.example.ipmedth_nfi.model.Hoofdthema
import com.example.ipmedth_nfi.model.Marker
import com.example.ipmedth_nfi.model.Observation
import com.example.ipmedth_nfi.model.Onderzoek
import com.example.ipmedth_nfi.model.ProjectStorage
import com.example.ipmedth_nfi.model.RoomModel
import com.example.ipmedth_nfi.ui.navigation.AssessmentPage
import kotlinx.coroutines.flow.MutableSharedFlow
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.SharingStarted
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asSharedFlow
import kotlinx.coroutines.flow.map
import kotlinx.coroutines.flow.stateIn
import kotlinx.coroutines.launch
import java.time.Instant

class SessionViewModel(
    application: Application,
    private val storage: ProjectStorage
) : AndroidViewModel(application) {

    // --- 1. Core State & Navigation ---
    private val _activeOnderzoek = MutableStateFlow<Onderzoek?>(null)
    val activeOnderzoek: StateFlow<Onderzoek?> = _activeOnderzoek

    val hasActiveOnderzoek: StateFlow<Boolean> = _activeOnderzoek
        .map { it != null }
        .stateIn(viewModelScope, SharingStarted.Eagerly, false)

    var currentAssessmentPage by mutableStateOf<AssessmentPage>(AssessmentPage.Info)
        private set

    val pageCompletion = mutableStateMapOf<AssessmentPage, Boolean>().apply {
        AssessmentPage.all.forEach { this[it] = false }
    }

    // --- 2. Data State (Snapshotted) ---
    val infoVooraf = mutableStateListOf<String>()
    val infoTerPlaatse = mutableStateListOf<String>()
    val observations = mutableStateListOf<Observation>()
    val hoofdthemas = mutableStateListOf<Hoofdthema>()
    val aandachtspunten = mutableStateListOf<Aandachtspunt>()
    val markers = mutableStateListOf<Marker>()
    val appData = mutableStateMapOf<String, String>()
    var roomModel: RoomModel? by mutableStateOf(null)

    private val _showNotification = MutableSharedFlow<String>()
    val showNotification = _showNotification.asSharedFlow()

    // --- 3. Lifecycle & Session Management ---
    fun startOnderzoek(onderzoek: Onderzoek) {
        _activeOnderzoek.value = onderzoek
        loadExistingSnapshot(onderzoek)
    }

    fun addBulletToAandachtspunt(id: String, bullet: String) {
        val index = aandachtspunten.indexOfFirst { it.id == id }
        if (index != -1) {
            val item = aandachtspunten[index]
            aandachtspunten[index] = item.copy(
                bulletPoints = item.bulletPoints + bullet
            )
            autoSave()
        }
    }

    fun deleteAandachtspunt(id: String) {
        aandachtspunten.removeAll { it.id == id }
        autoSave()
    }

    fun addAandachtspunt(
        title: String,
        theme: Hoofdthema
    ) {
        aandachtspunten += Aandachtspunt(
            title = title,
            theme = theme
        )
        autoSave()
    }

    fun updateAandachtspuntBullets(
        id: String,
        bullets: List<String>
    ) {
        val index = aandachtspunten.indexOfFirst { it.id == id }
        if (index != -1) {
            aandachtspunten[index] =
                aandachtspunten[index].copy(bulletPoints = bullets)
            autoSave()
        }
    }

    fun closeOnderzoek() {
        _activeOnderzoek.value = null
        // Optional: clear state here if you don't want it persisting in memory after close
    }
    fun setAssessmentPage(page: AssessmentPage) {
        currentAssessmentPage = page
    }

    // --- 4. Feature: Info Page ---
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

    // --- 5. Feature: Observations ---
    fun addObservation(beschrijving: String, locatie: String, notities: String) {
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

    fun toggleBookmark(id: String) {
        val index = observations.indexOfFirst { it.id == id }
        if (index != -1) {
            observations[index] = observations[index].copy(
                isBookmarked = !observations[index].isBookmarked
            )
        }
        autoSave()
    }

    // --- 6. Feature: Themes (Hoofdthemas) ---
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

    // --- 7. Feature: Media & Files ---
    fun createImageFile(): File {
        val onderzoek = _activeOnderzoek.value ?: error("No active onderzoek")
        val imageDir = storage.getImageDir(onderzoek)
        if (!imageDir.exists()) imageDir.mkdirs()

        return File(imageDir, "IMG_${UUID.randomUUID()}.jpg")
    }

    fun onPhotoCaptured(uri: Uri) {
        println("Captured photo: $uri")
        // TODO: Forward to ExportManager
        viewModelScope.launch {
            _showNotification.emit("Photo saved to: ${uri.lastPathSegment}")
        }
    }

    // --- 8. Persistence & Export Logic ---
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

    private fun loadExistingSnapshot(onderzoek: Onderzoek) {
        storage.loadSnapshot(onderzoek)?.let { snapshot ->
            infoVooraf.apply { clear(); addAll(snapshot.infoVooraf) }
            infoTerPlaatse.apply { clear(); addAll(snapshot.infoTerPlaatse) }
            observations.apply { clear(); addAll(snapshot.observations) }
            hoofdthemas.apply { clear(); addAll(snapshot.hoofdthemas) }
            aandachtspunten.apply { clear(); addAll(snapshot.aandachtspunten) }
            markers.apply { clear(); addAll(snapshot.markers) }
            appData.apply { clear(); putAll(snapshot.appData) }
            roomModel = snapshot.roomModel
        }
    }

    private fun autoSave() {
        _activeOnderzoek.value?.let { onderzoek ->
            try {
                storage.saveSnapshot(buildSnapshot(onderzoek))
            } catch (e: Exception) {
                e.printStackTrace()
            }
        }
    }

    private fun buildSnapshot(onderzoek: Onderzoek): ProjectSnapshot {
        return ProjectSnapshot(
            onderzoek = onderzoek,
            infoVooraf = infoVooraf.toList(),
            infoTerPlaatse = infoTerPlaatse.toList(),
            observations = observations.toList(),
            hoofdthemas = hoofdthemas.toList(),
            aandachtspunten = aandachtspunten.toList(),
            markers = markers.toList(),
            appData = appData.toMap(),
            roomModel = roomModel
        )
    }

    // Public accessor for the current snapshot to allow exporting
    fun getCurrentSnapshot(): ProjectSnapshot? {
        val onderzoek = _activeOnderzoek.value ?: return null
        return buildSnapshot(onderzoek)
    }

    fun buildExport(): ExportData {
        return ExportData(
            roomModel = roomModel,
            markers = markers.toList(),
            appData = appData.toMap()
        )
    }

    fun updateAandachtspunt(updated: com.example.ipmedth_nfi.model.Aandachtspunt) {
        val index = aandachtspunten.indexOfFirst { it.id == updated.id }
        if (index != -1) {
            aandachtspunten[index] = updated
            autoSave()
        }
    }

    // --- 9. Helpers: Detailed Aandachtspunt management ---
    // These helpers manage nested parts of Aandachtspunt (relevanteScenes, sceneProbabilities,
    // verwachteSporen, primaryActions, otherActions) and are defensive to avoid index errors.
    fun addRelevanteScene(aandachtId: String, scene: String) {
        val idx = aandachtspunten.indexOfFirst { it.id == aandachtId }
        if (idx != -1) {
            val item = aandachtspunten[idx]
            aandachtspunten[idx] = item.copy(relevanteScenes = item.relevanteScenes + scene)
            autoSave()
        }
    }

    fun removeRelevanteScene(aandachtId: String, sceneIndex: Int) {
        val idx = aandachtspunten.indexOfFirst { it.id == aandachtId }
        if (idx != -1) {
            val item = aandachtspunten[idx]
            if (sceneIndex in item.relevanteScenes.indices) {
                val newList = item.relevanteScenes.toMutableList().also { it.removeAt(sceneIndex) }
                aandachtspunten[idx] = item.copy(relevanteScenes = newList)
                autoSave()
            }
        }
    }

    fun updateRelevanteScenes(aandachtId: String, scenes: List<String>) {
        val idx = aandachtspunten.indexOfFirst { it.id == aandachtId }
        if (idx != -1) {
            val item = aandachtspunten[idx]
            aandachtspunten[idx] = item.copy(relevanteScenes = scenes)
            autoSave()
        }
    }

    fun setSceneProbabilities(aandachtId: String, probs: List<com.example.ipmedth_nfi.model.SceneProbability>) {
        val idx = aandachtspunten.indexOfFirst { it.id == aandachtId }
        if (idx != -1) {
            val item = aandachtspunten[idx]
            aandachtspunten[idx] = item.copy(sceneProbabilities = probs)
            autoSave()
        }
    }

    fun updateSceneProbability(aandachtId: String, scene: String, percent: Int) {
        val idx = aandachtspunten.indexOfFirst { it.id == aandachtId }
        if (idx == -1) return
        val item = aandachtspunten[idx]
        val updated = item.sceneProbabilities.map {
            if (it.scene == scene) it.copy(percent = percent) else it
        }
        aandachtspunten[idx] = item.copy(sceneProbabilities = updated)
        autoSave()
    }

    fun spreadProbabilitiesEqually(aandachtId: String) {
        val idx = aandachtspunten.indexOfFirst { it.id == aandachtId }
        if (idx == -1) return
        val item = aandachtspunten[idx]
        val size = item.relevanteScenes.size
        if (size == 0) return
        val base = 100 / size
        val remainder = 100 - (base * size)
        val probs = item.relevanteScenes.mapIndexed { i, s ->
            val extra = if (i == 0) remainder else 0 // add remainder to first to sum to 100
            com.example.ipmedth_nfi.model.SceneProbability(scene = s, percent = base + extra)
        }
        aandachtspunten[idx] = item.copy(sceneProbabilities = probs)
        autoSave()
    }

    fun setVerwachteSporen(aandachtId: String, verwachte: Map<String, List<String>>) {
        val idx = aandachtspunten.indexOfFirst { it.id == aandachtId }
        if (idx != -1) {
            val item = aandachtspunten[idx]
            aandachtspunten[idx] = item.copy(verwachteSporen = verwachte)
            autoSave()
        }
    }

    // Primary actions (Veiligstellen / Bemonsteren with optional subType and andersBeschrijving)
    fun addPrimaryAction(
        aandachtId: String,
        beschrijvingEnLocatie: String,
        type: com.example.ipmedth_nfi.model.ActionType,
        subType: com.example.ipmedth_nfi.model.SubActionType? = null,
        andersBeschrijving: String? = null
    ) {
        val idx = aandachtspunten.indexOfFirst { it.id == aandachtId }
        if (idx == -1) return
        val item = aandachtspunten[idx]
        val action = com.example.ipmedth_nfi.model.ActionItem(
            id = UUID.randomUUID().toString(),
            beschrijvingEnLocatie = beschrijvingEnLocatie,
            type = type,
            subType = subType,
            andersBeschrijving = andersBeschrijving
        )
        aandachtspunten[idx] = item.copy(primaryActions = item.primaryActions + action)
        autoSave()
    }

    fun updatePrimaryAction(aandachtId: String, updatedAction: com.example.ipmedth_nfi.model.ActionItem) {
        val idx = aandachtspunten.indexOfFirst { it.id == aandachtId }
        if (idx == -1) return
        val item = aandachtspunten[idx]
        val newList = item.primaryActions.map { if (it.id == updatedAction.id) updatedAction else it }
        aandachtspunten[idx] = item.copy(primaryActions = newList)
        autoSave()
    }

    fun deletePrimaryAction(aandachtId: String, actionId: String) {
        val idx = aandachtspunten.indexOfFirst { it.id == aandachtId }
        if (idx == -1) return
        val item = aandachtspunten[idx]
        val newList = item.primaryActions.filterNot { it.id == actionId }
        if (newList.size != item.primaryActions.size) {
            aandachtspunten[idx] = item.copy(primaryActions = newList)
            autoSave()
        }
    }

    // Other actions (top-level Anders entries)
    fun addOtherAction(aandachtId: String, beschrijvingEnLocatie: String) {
        val idx = aandachtspunten.indexOfFirst { it.id == aandachtId }
        if (idx == -1) return
        val item = aandachtspunten[idx]
        val action = com.example.ipmedth_nfi.model.ActionItem(
            id = UUID.randomUUID().toString(),
            beschrijvingEnLocatie = beschrijvingEnLocatie,
            type = com.example.ipmedth_nfi.model.ActionType.Anders,
            subType = null,
            andersBeschrijving = beschrijvingEnLocatie
        )
        aandachtspunten[idx] = item.copy(otherActions = item.otherActions + action)
        autoSave()
    }

    fun updateOtherAction(aandachtId: String, updatedAction: com.example.ipmedth_nfi.model.ActionItem) {
        val idx = aandachtspunten.indexOfFirst { it.id == aandachtId }
        if (idx == -1) return
        val item = aandachtspunten[idx]
        val newList = item.otherActions.map { if (it.id == updatedAction.id) updatedAction else it }
        aandachtspunten[idx] = item.copy(otherActions = newList)
        autoSave()
    }

    fun deleteOtherAction(aandachtId: String, actionId: String) {
        val idx = aandachtspunten.indexOfFirst { it.id == aandachtId }
        if (idx == -1) return
        val item = aandachtspunten[idx]
        val newList = item.otherActions.filterNot { it.id == actionId }
        if (newList.size != item.otherActions.size) {
            aandachtspunten[idx] = item.copy(otherActions = newList)
            autoSave()
        }
    }
}