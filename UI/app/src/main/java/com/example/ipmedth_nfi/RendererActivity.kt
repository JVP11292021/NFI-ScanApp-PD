package com.example.ipmedth_nfi

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.runtime.DisposableEffect
import androidx.compose.runtime.remember
import androidx.compose.ui.Modifier
import com.example.ipmedth_nfi.bridge.NativeAndroidEngine
import com.example.ipmedth_nfi.data.export.ProjectStorageManager
import com.example.ipmedth_nfi.model.Onderzoek
import com.example.ipmedth_nfi.ui.components.vk.VulkanSurface

class RendererActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        val zaaknummer = intent.getStringExtra("zaaknummer")
        val onderzoeksnaam = intent.getStringExtra("onderzoeksnaam")

        val projectPath = if (zaaknummer != null && onderzoeksnaam != null) {
            val onderzoek = Onderzoek(zaaknummer, onderzoeksnaam)
            val storageManager = ProjectStorageManager(applicationContext)
            storageManager.getProjectDirPath(onderzoek)
        } else {
            null
        }

        setContent {
            val engine = remember { NativeAndroidEngine() }

            DisposableEffect(Unit) {
                onDispose {
                    // The actual cleanup happens in surfaceDestroyed callback
                    // This ensures the engine is only created once per Activity
                }
            }

            VulkanSurface(
                modifier = Modifier.fillMaxSize(),
                engine = engine,
                projectDirPath = projectPath
            )
        }
    }
}

