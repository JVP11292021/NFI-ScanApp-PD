package com.example.ipmedth_nfi.model

import kotlinx.serialization.Serializable

@Serializable
data class RoomModel(
    val id: String = "",
    val description: String = ""
)