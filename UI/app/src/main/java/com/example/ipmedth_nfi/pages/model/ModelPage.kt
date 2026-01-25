package com.example.ipmedth_nfi.pages.model

import android.content.Intent
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.Button
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
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
    val activeOnderzoek = viewModel.activeOnderzoek.collectAsState().value

    Box(
        modifier = modifier.fillMaxSize(),
        contentAlignment = Alignment.Center
    ) {
        Button(
            onClick = {
                activeOnderzoek?.let { onderzoek ->
                    // Create an Intent to launch the RendererActivity
                    val intent = Intent(context, RendererActivity::class.java).apply {
                        putExtra("zaaknummer", onderzoek.zaaknummer)
                        putExtra("onderzoeksnaam", onderzoek.onderzoeksnaam)
                    }
                    context.startActivity(intent)
                }
            },
            enabled = activeOnderzoek != null
        ) {
            Text(text = "Open Vulkan Renderer")
        }
    }
}