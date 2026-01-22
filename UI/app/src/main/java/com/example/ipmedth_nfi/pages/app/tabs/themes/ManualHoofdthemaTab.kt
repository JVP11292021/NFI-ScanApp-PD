package com.example.ipmedth_nfi.pages.app.tabs.themes

import androidx.compose.foundation.layout.Column
import androidx.compose.material3.*
import androidx.compose.runtime.*
import com.example.ipmedth_nfi.model.Hoofdthema
import com.example.ipmedth_nfi.model.HoofdthemaType
import com.example.ipmedth_nfi.ui.components.thema.HoofdthemaDropdown
import com.example.ipmedth_nfi.viewmodel.SessionViewModel

@Composable
fun ManualHoofdthemaTab(
    viewModel: SessionViewModel,
    onDone: () -> Unit
) {
    var selected by remember { mutableStateOf<HoofdthemaType?>(null) }
    var relevant by remember { mutableStateOf(true) }
    var onderbouwing by remember { mutableStateOf("") }

    Column {
        HoofdthemaDropdown(
            selected = selected,
            onSelected = { selected = it }
        )

        SingleChoiceSegmentedButtonRow {
            SegmentedButton(
                selected = relevant,
                onClick = { relevant = true },
                shape = SegmentedButtonDefaults.itemShape(
                    index = 0,
                    count = 2
                )
            ) {
                Text("Relevant")
            }

            SegmentedButton(
                selected = !relevant,
                onClick = { relevant = false },
                shape = SegmentedButtonDefaults.itemShape(
                    index = 1,
                    count = 2
                )
            ) {
                Text("Niet relevant")
            }
        }

        TextField(
            value = onderbouwing,
            onValueChange = { onderbouwing = it },
            label = { Text("Onderbouwing") }
        )

        Button(
            enabled = selected != null,
            onClick = {
                viewModel.addHoofdthema(
                    Hoofdthema(
                        name = selected!!.displayName,
                        relevant = relevant,
                        onderbouwing = onderbouwing
                    )
                )
                onDone()
            }
        ) {
            Text("Toevoegen")
        }
    }
}