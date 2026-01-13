package com.example.ipmedth_nfi.model

import java.time.Instant
import java.util.UUID

data class Observation(
    val id: String = UUID.randomUUID().toString(),
    val createdAt: Instant,
    val beschrijving: String,
    val locatie: String,
    val notities: String,
    val isBookmarked: Boolean = false
)