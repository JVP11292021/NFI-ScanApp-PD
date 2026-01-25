package com.example.ipmedth_nfi.model

import kotlinx.serialization.Serializable
import java.util.UUID

@Serializable
data class Aandachtspunt(
    val id: String = UUID.randomUUID().toString(),
    val title: String,
    val theme: Hoofdthema,
    val bulletPoints: List<String> = emptyList()
)
