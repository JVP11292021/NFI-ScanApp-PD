package com.example.ipmedth_nfi.model

enum class DelictType (
    val displayName: String,
    val hoofdthemas: List<HoofdthemaType>
) {
    OVERLIJDENSONDERZOEK(
        displayName = "Overlijdensonderzoek",
        hoofdthemas = listOf(
            HoofdthemaType.MISDRIJF,
            HoofdthemaType.ZELFDODING,
            HoofdthemaType.ONGEVAL,
            HoofdthemaType.NATUURLIJK_OVERLIJDEN
        )
    ),
    BRANDSTICHTING(
        displayName = "Brandstichting",
        hoofdthemas = listOf(
            HoofdthemaType.OPZETTELIJK,
            HoofdthemaType.ONGEVAL
        )
    ),
    BINNENDRINGEN_WONING(
        displayName = "Binnendringen woning",
        hoofdthemas = listOf(
            HoofdthemaType.INSLUIPING_INKLIMMING,
            HoofdthemaType.INBRAAK,
            HoofdthemaType.DIEFSTAL_BRAAK,
            HoofdthemaType.DIEFSTAL_GEWELD,
            HoofdthemaType.VALSE_AANGIFTE
        )
    )
}