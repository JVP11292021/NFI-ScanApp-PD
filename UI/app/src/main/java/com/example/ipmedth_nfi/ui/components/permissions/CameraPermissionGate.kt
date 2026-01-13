package com.example.ipmedth_nfi.ui.components.permissions

import android.Manifest
import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import com.google.accompanist.permissions.*

@OptIn(ExperimentalPermissionsApi::class)
@Composable
fun CameraPermissionsGate(
    content: @Composable () -> Unit
) {
    val cameraPermission = rememberPermissionState(
        permission = Manifest.permission.CAMERA
    )

    when {
        cameraPermission.status.isGranted -> {
            content()
        }

        cameraPermission.status.shouldShowRationale -> {
            PermissionRationale(
                text = "Camera access is required to scan and capture images.",
                onRequest = { cameraPermission.launchPermissionRequest() }
            )
        }

        else -> {
            PermissionRequest(
                onRequest = { cameraPermission.launchPermissionRequest() }
            )
        }
    }
}

@Composable
private fun PermissionRequest(
    onRequest: () -> Unit
) {
    Box(modifier = Modifier.fillMaxSize(),
        contentAlignment = Alignment.Center
    ) {
        Button(onClick = onRequest) {
            Text("Allow camera access")
        }
    }
}

@Composable
private fun PermissionRationale(
    text: String,
    onRequest: () -> Unit
) {
    Box(
        modifier = Modifier.fillMaxSize(),
        contentAlignment = Alignment.Center
    ) {
        Column(horizontalAlignment = Alignment.CenterHorizontally) {
            Text(text)
            Spacer(modifier = Modifier.height(16.dp))
            Button(onClick = onRequest) {
                Text("Grant permission")
            }
        }
    }
}