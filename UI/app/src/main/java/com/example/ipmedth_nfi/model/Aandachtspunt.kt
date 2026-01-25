package com.example.ipmedth_nfi.model

import kotlinx.serialization.Serializable
import java.util.UUID

@Serializable
data class Aandachtspunt(
    val id: String = UUID.randomUUID().toString(),
    val title: String,
    val theme: Hoofdthema,
    val bulletPoints: List<String> = emptyList(),

    // New fields for Details modal
    val relevanteScenes: List<String> = emptyList(),
    val sceneProbabilities: List<SceneProbability> = emptyList(),
    val verwachteSporen: Map<String, List<String>> = emptyMap(),
    val primaryActions: List<ActionItem> = emptyList(),
    val otherActions: List<ActionItem> = emptyList()
)
