package com.example.ipmedth_nfi.pages.model

import android.content.Intent
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.Button
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import com.example.ipmedth_nfi.RendererActivity // Ensure this import matches your package
import com.example.ipmedth_nfi.viewmodel.SessionViewModel

@Composable
fun ModelPage(
    viewModel: SessionViewModel,
    modifier: Modifier = Modifier
) {
    val context = LocalContext.current

    Box(
        modifier = modifier.fillMaxSize(),
        contentAlignment = Alignment.Center
    ) {
        Button(onClick = {
            // Create an Intent to launch the RendererActivity
            val intent = Intent(context, RendererActivity::class.java)
            context.startActivity(intent)
        }) {
            Text(text = "Open Vulkan Renderer")
        }
    }
}