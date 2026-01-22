package com.example.ipmedth_nfi.pages.app.tabs.themes

import androidx.compose.foundation.layout.Column
import androidx.compose.material3.Button
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import com.example.ipmedth_nfi.model.DelictType
import com.example.ipmedth_nfi.model.Hoofdthema
import com.example.ipmedth_nfi.viewmodel.SessionViewModel

@Composable
fun DelictTab(
    viewModel: SessionViewModel,
    onDone: () -> Unit
) {
    Column {
        DelictType.entries.forEach { delict ->
            Button(
                onClick = {
                    delict.hoofdthemas.forEach {
                        viewModel.addHoofdthema(
                            Hoofdthema(
                                name = it.displayName,
                                relevant = true,
                                onderbouwing = ""
                            )
                        )
                    }
                    onDone()
                }
            ) {
                Text(delict.displayName)
            }
        }
    }
}
