package com.example.ipmedth_nfi.model

import kotlinx.serialization.Serializable

@Serializable
data class RoomModel(
    val id: String = "",
    val description: String = "",
    val rotationOffsetX: Float = 0f,
    val rotationOffsetY: Float = 0f,
    val rotationOffsetZ: Float = 0f
)