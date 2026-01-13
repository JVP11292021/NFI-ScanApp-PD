package com.example.ipmedth_nfi.pages.app.tabs

import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import com.example.ipmedth_nfi.ui.components.InfoSubheader
import com.example.ipmedth_nfi.viewmodel.SessionViewModel

@Composable
fun ThemesTab(viewModel: SessionViewModel) {
    var showAddDialog by remember { mutableStateOf(false) }
    Column(modifier = Modifier.fillMaxSize()) {
        InfoSubheader(
            "Hoofdthema's",
            "X thema's",
            "+ Thema",
            onAddClick = { showAddDialog = true }
        )
    }
}