package com.example.ipmedth_nfi.ui.navigation

import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Check
import androidx.compose.material.icons.filled.Info
import androidx.compose.material.icons.filled.List
import androidx.compose.material.icons.filled.Visibility
import androidx.compose.material.icons.filled.Warning
import androidx.compose.material.icons.outlined.CheckCircle
import androidx.compose.material.icons.outlined.Info
import androidx.compose.material.icons.outlined.Warning
import androidx.compose.ui.graphics.vector.ImageVector
import com.composables.ClipboardList
import com.composables.EyeOpen24
import com.composables.Stacks

enum class AssessmentPage(
    val title: String,
    val icon: ImageVector
) {
    Info("Info", Icons.Default.Info),
    Observations("Observations", Icons.Default.Visibility),
    Themes("Themes", Icons.Default.List),
    Attention("Attention", Icons.Default.Warning),
    Plan("Plan", Icons.Default.Check);

    companion object {
        val pages = entries
    }
}
