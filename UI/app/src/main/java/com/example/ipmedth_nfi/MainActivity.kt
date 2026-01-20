package com.example.ipmedth_nfi

import android.app.Application
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.ui.platform.LocalContext
import androidx.lifecycle.ViewModel
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.viewmodel.compose.viewModel
import com.example.ipmedth_nfi.data.export.ProjectStorageManager
import com.example.ipmedth_nfi.navigation.AppEntryPoint
import com.example.ipmedth_nfi.ui.theme.IPMEDTH_NFITheme
import com.example.ipmedth_nfi.viewmodel.AppViewModel
import com.example.ipmedth_nfi.viewmodel.SessionViewModel
import com.example.ipmedth_nfi.viewmodel.SessionViewModelFactory

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()

        setContent {
            IPMEDTH_NFITheme {
                val context = LocalContext.current

                val appViewModel: AppViewModel = viewModel(
                    factory = object : ViewModelProvider.Factory {
                        @Suppress("UNCHECKED_CAST")
                        override fun <T : ViewModel> create(modelClass: Class<T>): T {
                            val storage = ProjectStorageManager(context.applicationContext)
                            return AppViewModel(storage) as T
                        }
                    }
                )

                val sessionViewModel: SessionViewModel = viewModel(
                    factory = SessionViewModelFactory(
                        context.applicationContext as Application
                    )
                )

                AppEntryPoint(
                    sessionViewModel = sessionViewModel,
                    appViewModel = appViewModel
                )
            }
        }
    }
}