package com.example.ipmedth_nfi.pages.scan

import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import com.example.ipmedth_nfi.ui.components.permissions.CameraPermissionsGate
import com.example.ipmedth_nfi.viewmodel.SessionViewModel

@Composable
fun ScanPage(
    viewModel: SessionViewModel,
    modifier: Modifier = Modifier
) {
    CameraPermissionsGate {
        ScanCameraContent(
            viewModel = viewModel,
            modifier = modifier
        )
    }
}