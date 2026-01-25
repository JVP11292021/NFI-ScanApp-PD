package com.example.ipmedth_nfi.pages.app.tabs

import android.os.Build
import androidx.annotation.RequiresApi
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import com.example.ipmedth_nfi.ui.components.observations.AddObservationDialog
import com.example.ipmedth_nfi.ui.components.InfoSubheader
import com.example.ipmedth_nfi.ui.components.observations.ObservationCard
import com.example.ipmedth_nfi.ui.components.observations.ObservationSearchBar
import com.example.ipmedth_nfi.viewmodel.SessionViewModel

@RequiresApi(Build.VERSION_CODES.O)
@Composable
fun ObservationsTab(
    viewModel: SessionViewModel
) {
    var showAddDialog by remember { mutableStateOf(false) }
    var searchQuery by remember { mutableStateOf("") }
    val scrollState = rememberScrollState()

    Column(Modifier
        .fillMaxSize()
        .verticalScroll(scrollState)
    ) {
        val totalCount = viewModel.observations.size
        val bookmarkCount = viewModel.observations.count{ it.isBookmarked }

        InfoSubheader(
            subject = "Observations",
            details = "$totalCount totaal - $bookmarkCount bookmark(s)",
            actionTitle = "+ Nieuw",
            onAddClick = { showAddDialog = true }
        )

        ObservationSearchBar(
            query = searchQuery,
            onQueryChange = { searchQuery = it }
        )

        val filteredObservations = viewModel.observations.filter {
            it.beschrijving.contains(searchQuery, ignoreCase = true) ||
            it.locatie.contains(searchQuery, ignoreCase = true) ||
            it.notities.contains(searchQuery, ignoreCase = true)
        }

        LazyColumn(
            modifier = Modifier.weight(1f),
            verticalArrangement = Arrangement.spacedBy(16.dp)
        ) {
            items(filteredObservations, key = { it.id }) { observation ->
                ObservationCard (
                    observation = observation,
                    onDelete = { viewModel.deleteObservation(observation.id) },
                    onToggleBookmark = { viewModel.toggleBookmark(observation.id) },
                    onAddImage = {
                        //TODO: Hook into ScanPage
                    }
                )
            }
        }
    }

    if (showAddDialog) {
        AddObservationDialog(
            onConfirm = { beschrijving, locatie, notitites ->
                viewModel.addObservation(beschrijving, locatie, notitites)
                showAddDialog = false
            },
            onDismiss = { showAddDialog = false }
        )
    }
}