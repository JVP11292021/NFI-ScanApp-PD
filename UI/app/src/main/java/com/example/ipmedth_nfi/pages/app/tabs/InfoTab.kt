package com.example.ipmedth_nfi.pages.app.tabs

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.ModalBottomSheet
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.RadioButton
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.material3.rememberModalBottomSheetState
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp
import com.example.ipmedth_nfi.ui.components.InfoDropdown
import com.example.ipmedth_nfi.ui.components.InfoSubheader
import com.example.ipmedth_nfi.viewmodel.SessionViewModel

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun InfoTab(viewModel: SessionViewModel) {
    var showBottomSheet by remember { mutableStateOf(false) }
    
    // Live counts
    val countVooraf = viewModel.infoVooraf.size
    val countTerPlaatse = viewModel.infoTerPlaatse.size
    val scrollState = rememberScrollState()

    Column(
        modifier = Modifier
            .fillMaxSize()
            .verticalScroll(scrollState)) {
        InfoSubheader(
            subject = "Informatie", 
            details = "$countVooraf vooraf - $countTerPlaatse ter plaatse",
            actionTitle = "+ Nieuw",
            onAddClick = { showBottomSheet = true }
        )
        
        Spacer(Modifier.size(8.dp))
        
        InfoDropdown(
            title = "Vooraf",
            items = viewModel.infoVooraf,
            onDeleteItem = { index ->
                viewModel.removeInfoVooraf(index)
            }
        )
        
        Spacer(Modifier.size(8.dp))
        
        InfoDropdown(
            title = "Ter Plaatse", 
            items = viewModel.infoTerPlaatse,
            onDeleteItem = { index ->
                viewModel.removeInfoTerPlaatse(index)
            }
        )
    }

    if (showBottomSheet) {
        ModalBottomSheet(
            onDismissRequest = { showBottomSheet = false },
            sheetState = rememberModalBottomSheetState()
        ) {
            AddItemPopupContent(
                onDismiss = { showBottomSheet = false },
                onAdd = { text, isVooraf ->
                    if (isVooraf) {
                        viewModel.addInfoVooraf(text)
                    } else {
                        viewModel.addInfoTerPlaatse(text)
                    }
                    showBottomSheet = false
                }
            )
        }
    }
}

@Composable
fun AddItemPopupContent(onDismiss: () -> Unit, onAdd: (String, Boolean) -> Unit) {
    var text by remember { mutableStateOf("") }
    var isVooraf by remember { mutableStateOf(true) }

    Column(
        modifier = Modifier
            .fillMaxWidth()
            .padding(16.dp)
            .padding(bottom = 32.dp) // Extra padding for bottom sheet handling
    ) {
        Text(
            text = "Informatie",
            style = MaterialTheme.typography.titleLarge,
            modifier = Modifier.padding(bottom = 16.dp)
        )

        // Toggle (RadioButton approach for simplicity to switch between two options)
        Row(
            verticalAlignment = Alignment.CenterVertically,
            modifier = Modifier.fillMaxWidth()
        ) {
            Row(verticalAlignment = Alignment.CenterVertically) {
                RadioButton(selected = isVooraf, onClick = { isVooraf = true })
                Text("Vooraf", modifier = Modifier.padding(start = 4.dp))
            }
            Spacer(modifier = Modifier.width(16.dp))
            Row(verticalAlignment = Alignment.CenterVertically) {
                RadioButton(selected = !isVooraf, onClick = { isVooraf = false })
                Text("Ter Plaatse", modifier = Modifier.padding(start = 4.dp))
            }
        }

        Spacer(modifier = Modifier.height(16.dp))

        OutlinedTextField(
            value = text,
            onValueChange = { text = it },
            label = { Text("Beschrijving") },
            modifier = Modifier.fillMaxWidth(),
            singleLine = true
        )

        Spacer(modifier = Modifier.height(24.dp))

        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.End
        ) {
            TextButton(onClick = onDismiss) {
                Text("Annuleren")
            }
            Spacer(modifier = Modifier.width(8.dp))
            Button(
                onClick = { onAdd(text, isVooraf) },
                enabled = text.isNotBlank(),
                colors = ButtonDefaults.buttonColors(containerColor = Color(0xFF0066CC))
            ) {
                Text("Toevoegen")
            }
        }
    }
}
