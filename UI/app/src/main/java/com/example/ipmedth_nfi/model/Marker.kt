package com.example.ipmedth_nfi.model

import kotlinx.serialization.Serializable

@Serializable
data class Marker(
    val id: String,
    val label: String,
    val x: Float,
    val y: Float,
    val z: Float
)