package com.example.ipmedth_nfi.pages.model

import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.remember
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import com.example.ipmedth_nfi.bridge.NativeAndroidEngine
import com.example.ipmedth_nfi.ui.components.vk.VulkanSurface
import com.example.ipmedth_nfi.viewmodel.SessionViewModel

@Composable
fun ModelPage(
    viewModel: SessionViewModel,
    modifier: Modifier = Modifier
) {
    val engine = remember { NativeAndroidEngine() }

    Box(modifier = modifier.fillMaxSize()) {
        VulkanSurface(
            modifier = Modifier.fillMaxSize(),
            engine = engine
        )
    }
}
