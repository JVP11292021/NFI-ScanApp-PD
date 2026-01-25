package com.example.ipmedth_nfi.model

import kotlinx.serialization.Serializable

@Serializable
enum class ActionType { Veiligstellen, Bemonsteren, Anders }

@Serializable
enum class SubActionType { Epitheel, Speeksel, Bloed, Sperma, Anders }

@Serializable
data class ActionItem(
    val id: String,
    val beschrijvingEnLocatie: String,
    val type: ActionType,
    val subType: SubActionType? = null,
    val andersBeschrijving: String? = null
)
