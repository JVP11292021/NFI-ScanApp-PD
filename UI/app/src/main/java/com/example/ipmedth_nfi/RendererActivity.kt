package com.example.ipmedth_nfi

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.runtime.remember
import androidx.compose.ui.Modifier
import com.example.ipmedth_nfi.bridge.NativeAndroidEngine
import com.example.ipmedth_nfi.ui.components.vk.VulkanSurface

class RendererActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        setContent {
            // Instantiate and remember the engine to keep it across recompositions
            val engine = remember { NativeAndroidEngine() }

            VulkanSurface(
                modifier = Modifier.fillMaxSize(),
                engine = engine
            )
        }
    }
}