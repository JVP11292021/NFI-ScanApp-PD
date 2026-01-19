package com.example.ipmedth_nfi.ui.components.navbar

import androidx.compose.foundation.layout.padding
import androidx.compose.material3.ModalDrawerSheet
import androidx.compose.material3.NavigationDrawerItem
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import com.example.ipmedth_nfi.navigation.MainRoute

@Composable
fun AppDrawer(onSelect: (String) -> Unit) {
    ModalDrawerSheet() {
        Text("Main Pages", modifier = Modifier.padding(16.dp))

        MainRoute.entries.forEach { route ->
            NavigationDrawerItem(
                label = { Text(route.title)},
                selected = false,
                onClick = { onSelect(route.route)}
            )
        }
    }
}