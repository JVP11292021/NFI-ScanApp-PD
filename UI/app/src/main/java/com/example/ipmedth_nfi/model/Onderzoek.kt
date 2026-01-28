package com.example.ipmedth_nfi.model

import kotlinx.serialization.Serializable
import java.util.UUID

@Serializable
data class Onderzoek(
    val zaaknummer: String,
    val onderzoeksnaam: String,
    val createdAt: Long = System.currentTimeMillis(),
    val internalId: String = UUID.randomUUID().toString()
) {
    val projectId: String
        get() = internalId
}
