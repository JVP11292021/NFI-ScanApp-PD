package com.example.ipmedth_nfi.pages.app.tabs

import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.size
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import com.example.ipmedth_nfi.ui.components.InfoDropdown
import com.example.ipmedth_nfi.ui.components.InfoSubheader
import com.example.ipmedth_nfi.viewmodel.SessionViewModel

@Composable
fun InfoTab(viewMap: SessionViewModel) {
    Column(modifier = Modifier.fillMaxSize()) {
        InfoSubheader(subject = "Informatie", details = "X vooraf - Y ter plaatse")
        Spacer(Modifier.size(8.dp))
        InfoDropdown(title = "Vooraf")
        Spacer(Modifier.size(8.dp))
        InfoDropdown(title = "Ter Plaatse")
    }
}
