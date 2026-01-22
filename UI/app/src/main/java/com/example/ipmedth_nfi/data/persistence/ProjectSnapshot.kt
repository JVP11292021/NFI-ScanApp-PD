package com.example.ipmedth_nfi.data.persistence

import com.example.ipmedth_nfi.model.Hoofdthema
import com.example.ipmedth_nfi.model.Marker
import com.example.ipmedth_nfi.model.Observation
import com.example.ipmedth_nfi.model.Onderzoek
import com.example.ipmedth_nfi.model.RoomModel
import kotlinx.serialization.Serializable

@Serializable
data class ProjectSnapshot(
    val onderzoek: Onderzoek,
    val infoVooraf: List<String> = emptyList(),
    val infoTerPlaatse: List<String> = emptyList(),
    val observations: List<Observation> = emptyList(),
    val hoofdthemas: List<Hoofdthema> = emptyList(),
    val markers: List<Marker> = emptyList(),
    val appData: Map<String, String> = emptyMap(),
    val roomModel: RoomModel? = null
) {
    companion object {
        fun empty(onderzoek: Onderzoek): ProjectSnapshot =
            ProjectSnapshot(onderzoek = onderzoek)
    }
}
