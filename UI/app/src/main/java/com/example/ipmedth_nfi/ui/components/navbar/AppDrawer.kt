package com.example.ipmedth_nfi.ui.components.navbar

import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxHeight
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.ModalDrawerSheet
import androidx.compose.material3.NavigationDrawerItem
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import com.example.ipmedth_nfi.navigation.MainRoute

@Composable
fun AppDrawer(
    onMainRouteClick: (String) -> Unit,
    onExportClick: () -> Unit,
    onSaveAndClose: () -> Unit
) {
    ModalDrawerSheet {
        Column(Modifier.fillMaxHeight()) {

            MainRoute.entries.forEach { route ->
                NavigationDrawerItem(
                    label = { Text(route.title) },
                    selected = false,
                    onClick = { onMainRouteClick(route.route) }
                )
            }

            Spacer(Modifier.weight(1f))

            NavigationDrawerItem(
                label = { Text("Finish & Export") },
                selected = false,
                onClick = onExportClick
            )

            NavigationDrawerItem(
                label = { Text("Save & Close Project") },
                selected = false,
                onClick = onSaveAndClose
            )
        }
    }
}
