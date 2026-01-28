package com.example.ipmedth_nfi.pages.app.tabs

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import com.example.ipmedth_nfi.ui.components.InfoSubheader
import com.example.ipmedth_nfi.ui.components.thema.AddHoofdthemaChooserDialog
import com.example.ipmedth_nfi.ui.components.thema.AddHoofdthemaDialog
import com.example.ipmedth_nfi.ui.components.thema.AddHoofdthemaFromDelictDialog
import com.example.ipmedth_nfi.ui.components.thema.HoofdthemaCard
import com.example.ipmedth_nfi.viewmodel.SessionViewModel

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun ThemesTab(viewModel: SessionViewModel) {
    var showAddSheet by remember { mutableStateOf(false) }

    Column(modifier = Modifier.fillMaxSize()) {

        val totaalCount = viewModel.hoofdthemas.size
        val relevantCount = viewModel.relevantCount()
        var showChooser by remember { mutableStateOf(false) }
        var showManual by remember { mutableStateOf(false) }
        var showDelict by remember { mutableStateOf(false) }


        InfoSubheader(
            "Hoofdthema's",
            "$totaalCount thema's - $relevantCount relevant",
            "+ Thema",
            onAddClick = { showChooser = true }
        )
        Spacer(Modifier.padding(8.dp))
        LazyColumn(
            verticalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            items(viewModel.hoofdthemas) { thema ->
                HoofdthemaCard(
                    thema = thema,
                    onDelete = { viewModel.deleteHoofdthema(thema.id) },
                    onUpdate = { viewModel.updateHoofdthema(it) }
                )
            }
        }

        if (showChooser) {
            AddHoofdthemaChooserDialog(
                onManual = {
                    showChooser = false
                    showManual = true
                },
                onDelict = {
                    showChooser = false
                    showDelict = true
                },
                onDismiss = { showChooser = false }
            )
        }

        if (showManual) {
            AddHoofdthemaDialog(
                onConfirm = {
                    viewModel.addHoofdthema(it)
                    showManual = false
                },
                onDismiss = { showManual = false }
            )
        }

        if (showDelict) {
            AddHoofdthemaFromDelictDialog(
                onConfirm = { themas ->
                    themas.forEach(viewModel::addHoofdthema)
                    showDelict = false
                },
                onDismiss = { showDelict = false }
            )
        }
    }
}
