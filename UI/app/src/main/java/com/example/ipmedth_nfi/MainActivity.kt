package com.example.ipmedth_nfi

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.lifecycle.viewmodel.compose.viewModel
import com.example.ipmedth_nfi.navigation.AppEntryPoint
import com.example.ipmedth_nfi.ui.theme.IPMEDTH_NFITheme
import com.example.ipmedth_nfi.viewmodel.SessionViewModel

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContent {
            IPMEDTH_NFITheme {
                val sessionViewModel: SessionViewModel = viewModel()
                AppEntryPoint(sessionViewModel)
            }
        }
    }
}