package com.example.ipmedth_nfi.ui.components.permissions

import android.Manifest
import android.app.Activity
import android.content.Context
import android.content.ContextWrapper
import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.unit.dp
import com.google.accompanist.permissions.ExperimentalPermissionsApi
import com.google.accompanist.permissions.isGranted
import com.google.accompanist.permissions.rememberPermissionState
import com.google.accompanist.permissions.shouldShowRationale

@OptIn(ExperimentalPermissionsApi::class)
@Composable
fun CameraPermissionsGate(
    content: @Composable () -> Unit
) {
    val context = LocalContext.current
    val activity = remember(context) { context.findActivity() }

    val cameraPermissionState = rememberPermissionState(
        permission = Manifest.permission.CAMERA
    )

    when {
        cameraPermissionState.status.isGranted -> {
            content()
        }

        cameraPermissionState.status.shouldShowRationale -> {
            PermissionRationale(
                text = "Camera access is required to scan and capture images.",
                onRequest = {
                    activity.runOnUiThread {
                        cameraPermissionState.launchPermissionRequest()
                    }
                }
            )
        }

        else -> {
            PermissionRequest(
                onRequest = {
                    activity.runOnUiThread {
                        cameraPermissionState.launchPermissionRequest()
                    }
                }
            )
        }
    }
}

@Composable
private fun PermissionRequest(
    onRequest: () -> Unit
) {
    Box(
        modifier = Modifier.fillMaxSize(),
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

/**
 * Safely extract the hosting Activity from a Context.
 * Required to ensure permission requests are bound to a valid Activity.
 */
private fun Context.findActivity(): Activity {
    var ctx = this
    while (ctx is ContextWrapper) {
        if (ctx is Activity) return ctx
        ctx = ctx.baseContext
    }
    error("Activity not found in context chain.")
}
