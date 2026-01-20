package com.example.ipmedth_nfi.model

import kotlinx.serialization.Serializable

@Serializable
data class Onderzoek(
    val zaaknummer: String,
    val onderzoeksnaam: String,
    val createdAt: Long = System.currentTimeMillis(),
) {
    val projectId: String
        get() = "${zaaknummer}_${onderzoeksnaam}"

    val basepath: String
        get() = "NFI_Scanapp/$zaaknummer.$onderzoeksnaam"
}