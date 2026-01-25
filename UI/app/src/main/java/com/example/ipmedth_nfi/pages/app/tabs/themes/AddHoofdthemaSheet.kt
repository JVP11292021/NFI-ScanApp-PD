package com.example.ipmedth_nfi.pages.app.tabs.themes

import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.ModalBottomSheet
import androidx.compose.material3.SecondaryTabRow
import androidx.compose.material3.Tab
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import com.example.ipmedth_nfi.viewmodel.SessionViewModel

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun AddHoofdthemaSheet(
    viewModel: SessionViewModel,
    onDismiss: () -> Unit,
) {
    var tab by remember { mutableStateOf(0) }

    ModalBottomSheet(
        onDismissRequest = onDismiss
    ) {
        SecondaryTabRow(
            selectedTabIndex = tab
        ) {
            Tab(
                selected = tab == 0,
                onClick = { tab = 0 }
            ) {
                Text("Type delict")
            }

            Tab(
                selected = tab == 1,
                onClick = { tab = 1 }
            ) {
                Text("Hoofdthema toevoegen")
            }
        }

        when (tab) {
            0 -> DelictTab(
                viewModel = viewModel,
                onDone = onDismiss
            )

            1 -> ManualHoofdthemaTab(
                viewModel = viewModel,
                onDone = onDismiss
            )
        }
    }
}
