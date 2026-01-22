package com.example.ipmedth_nfi.ui.components.thema

import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.ExposedDropdownMenuAnchorType
import androidx.compose.material3.ExposedDropdownMenuBox
import androidx.compose.material3.ExposedDropdownMenuDefaults
import androidx.compose.material3.Text
import androidx.compose.material3.TextField
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import com.example.ipmedth_nfi.model.HoofdthemaType

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun HoofdthemaDropdown(
    selected: HoofdthemaType?,
    onSelected: (HoofdthemaType) -> Unit
) {
    var expanded by remember { mutableStateOf(false) }

    ExposedDropdownMenuBox(
        expanded = expanded,
        onExpandedChange = { expanded = !expanded }
    ) {
        val fillMaxWidth = Modifier
            .fillMaxWidth()
        TextField(
            modifier = fillMaxWidth.menuAnchor(type = ExposedDropdownMenuAnchorType.PrimaryEditable, enabled = true),
            value = selected?.displayName ?: "",
            onValueChange = {},
            readOnly = true,
            label = { Text("Hoofdthema") },
            trailingIcon = {
                ExposedDropdownMenuDefaults.TrailingIcon(expanded)
            },
            colors = ExposedDropdownMenuDefaults.textFieldColors()
        )

        ExposedDropdownMenu(
            expanded = expanded,
            onDismissRequest = { expanded = false }
        ) {
            HoofdthemaType.entries.forEach { type ->
                DropdownMenuItem(
                    text = { Text(type.displayName) },
                    onClick = {
                        onSelected(type)
                        expanded = false
                    }
                )
            }
        }
    }
}
