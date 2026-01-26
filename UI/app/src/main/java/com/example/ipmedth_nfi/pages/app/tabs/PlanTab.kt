package com.example.ipmedth_nfi.pages.app.tabs

import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import com.example.ipmedth_nfi.ui.components.plan.PlanCard
import com.example.ipmedth_nfi.viewmodel.SessionViewModel

@Composable
fun PlanTab(viewModel: SessionViewModel, onNavigateToAnnotation: (String) -> Unit = {}) {
    val count = viewModel.aandachtspunten.size

    Column(Modifier.fillMaxSize().padding(8.dp)) {
        Text(text = "Werkplan — $count totaal", modifier = Modifier.padding(bottom = 8.dp))

        viewModel.aandachtspunten.forEach { item ->
            PlanCard(
                item = item,
                onEditAction = { /* TODO: open edit */ },
                onScanSinForAction = { /* TODO: scan SIN */ },
                onNavigateToAnnotation = onNavigateToAnnotation
            )
        }
    }
}