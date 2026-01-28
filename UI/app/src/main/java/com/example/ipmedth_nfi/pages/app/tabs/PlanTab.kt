package com.example.ipmedth_nfi.pages.app.tabs

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import com.example.ipmedth_nfi.ui.components.plan.PlanCard
import com.example.ipmedth_nfi.viewmodel.SessionViewModel

@Composable
fun PlanTab(viewModel: SessionViewModel, onNavigateToAnnotation: (String) -> Unit = {}) {
    LazyColumn(
        modifier = Modifier.fillMaxSize(),
        verticalArrangement = Arrangement.spacedBy(12.dp)
    ) {
        items(viewModel.aandachtspunten, key = { it.id }) { item ->
            PlanCard(
                item = item,
                onEditAction = { /* TODO: open edit */ },
                onScanSinForAction = { /* TODO: scan SIN */ },
                onNavigateToAnnotation = onNavigateToAnnotation
            )
        }
    }
}