package com.example.ipmedth_nfi.ui.navigation

import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.outlined.List
import androidx.compose.material.icons.filled.Check
import androidx.compose.material.icons.filled.Info
import androidx.compose.material.icons.filled.List
import androidx.compose.material.icons.filled.Visibility
import androidx.compose.material.icons.filled.Warning
import androidx.compose.material.icons.outlined.Check
import androidx.compose.material.icons.outlined.CheckCircle
import androidx.compose.material.icons.outlined.Info
import androidx.compose.material.icons.outlined.List
import androidx.compose.material.icons.outlined.Visibility
import androidx.compose.material.icons.outlined.Warning
import androidx.compose.ui.graphics.vector.ImageVector
import com.composables.ClipboardList
import com.composables.EyeOpen24
import com.composables.Stacks

enum class AssessmentPage(
    val title: String,
    val icon: ImageVector
) {
    Info("Info", Icons.Outlined.Info),
    Observations("Observations", Icons.Outlined.Visibility),
    Themes("Themes", Icons.AutoMirrored.Outlined.List),
    Attention("Attention", Icons.Outlined.Warning),
    Plan("Plan", Icons.Outlined.Check);

    companion object {
        val pages = entries
    }
}
