package com.example.ipmedth_nfi.model

data class ExportData(
    val roomModel: RoomModel?,
    val markers: List<Marker>?,
    val appData: Map<String, String>
)