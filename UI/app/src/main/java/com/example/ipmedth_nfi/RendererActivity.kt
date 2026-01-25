package com.example.ipmedth_nfi

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.runtime.remember
import androidx.compose.ui.Modifier
import com.example.ipmedth_nfi.bridge.NativeAndroidEngine
import com.example.ipmedth_nfi.data.export.ProjectStorageManager
import com.example.ipmedth_nfi.model.Onderzoek
import com.example.ipmedth_nfi.ui.components.vk.VulkanSurface

class RendererActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        // Extract project information from intent
        val zaaknummer = intent.getStringExtra("zaaknummer")
        val onderzoeksnaam = intent.getStringExtra("onderzoeksnaam")

        // Create Onderzoek object and get project path
        val projectPath = if (zaaknummer != null && onderzoeksnaam != null) {
            val onderzoek = Onderzoek(zaaknummer, onderzoeksnaam)
            val storageManager = ProjectStorageManager(applicationContext)
            storageManager.getProjectDirPath(onderzoek)
        } else {
            null
        }

        setContent {
            // Instantiate and remember the engine to keep it across recompositions
            val engine = remember { NativeAndroidEngine() }

            VulkanSurface(
                modifier = Modifier.fillMaxSize(),
                engine = engine,
                projectDirPath = projectPath
            )
        }
    }
}

