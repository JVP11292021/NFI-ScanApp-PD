package com.example.ipmedth_nfi.model

import kotlinx.serialization.Serializable

@Serializable
data class SceneProbability(
    val scene: String,
    val percent: Int = 0,
    val colorHex: String = "#${Integer.toHexString(android.graphics.Color.rgb(100,150,200) and 0xFFFFFF)}"
)
