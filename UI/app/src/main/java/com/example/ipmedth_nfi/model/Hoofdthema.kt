package com.example.ipmedth_nfi.model

import kotlinx.serialization.Serializable
import java.util.UUID

@Serializable
data class Hoofdthema(
    val id: String = UUID.randomUUID().toString(),
    val name: String,
    val relevant: Boolean,
    val onderbouwing: String,
    val mogelijkheden: List<String> = emptyList()
)