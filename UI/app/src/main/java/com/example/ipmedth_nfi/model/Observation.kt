package com.example.ipmedth_nfi.model

import kotlinx.serialization.Serializable
import java.time.Instant
import java.util.UUID

@Serializable
data class Observation(
    val id: String = UUID.randomUUID().toString(),
    @Serializable(with = InstantSerializer::class)
    val createdAt: Instant,
    val beschrijving: String,
    val locatie: String,
    val notities: String,
    val isBookmarked: Boolean = false
)