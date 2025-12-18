package com.example.ipmedth_nfi.ui.navigation

import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.outlined.CheckCircle
import androidx.compose.material.icons.outlined.Info
import androidx.compose.material.icons.outlined.Warning
import androidx.compose.ui.graphics.vector.ImageVector
import com.composables.ClipboardList
import com.composables.EyeOpen24
import com.composables.Stacks

sealed class AssessmentPage(
    val title: String,
    val icon: ImageVector
) {
    object Info : AssessmentPage("Info", Icons.Outlined.Info)
    object Observations : AssessmentPage("Observations", EyeOpen24)
    object Themes : AssessmentPage("Themes", Stacks)
    object Attention : AssessmentPage("Attention", Icons.Outlined.Warning)
    object Plan : AssessmentPage("Plan", ClipboardList)
    object Finish : AssessmentPage("Finish", Icons.Outlined.CheckCircle)

    companion object {
        val all = listOf(
            Info,
            Observations,
            Themes,
            Attention,
            Plan,
            Finish
        )
    }
}