package com.example.ipmedth_nfi.pages.app.tabs

import androidx.compose.animation.AnimatedVisibility
import androidx.compose.animation.ExperimentalAnimationApi
import androidx.compose.animation.fadeIn
import androidx.compose.animation.fadeOut
import androidx.compose.animation.slideInVertically
import androidx.compose.animation.slideOutVertically
import androidx.compose.foundation.horizontalScroll
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.rememberScrollState
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.ModalBottomSheet
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import com.example.ipmedth_nfi.ui.components.InfoSubheader
import com.example.ipmedth_nfi.ui.components.attention.AddAandachtspuntSheet
import com.example.ipmedth_nfi.ui.components.attention.AttentionCard
import com.example.ipmedth_nfi.ui.components.attention.AandachtspuntDetailsSheet
import com.example.ipmedth_nfi.ui.components.attention.ThemeMiniCard
import com.example.ipmedth_nfi.viewmodel.SessionViewModel

@OptIn(ExperimentalMaterial3Api::class, ExperimentalAnimationApi::class)
@Composable
fun AttentionTab(viewModel: SessionViewModel) {
    val themes = viewModel.hoofdthemas
    val aandachtspunten = viewModel.aandachtspunten
    var showAddSheet by remember { mutableStateOf(false) }

    var selectedDetailsItemId by remember { mutableStateOf<String?>(null) }

    val selected = remember(aandachtspunten, selectedDetailsItemId) {
        aandachtspunten.find { it.id == selectedDetailsItemId }
    }

    Column(modifier = Modifier.fillMaxSize()) {
        InfoSubheader(
            subject = "Aandachtspunten",
            details = "${aandachtspunten.size} totaal",
            actionTitle = "+ Nieuw",
            onAddClick = { showAddSheet = true }
        )

        Row(
            modifier = Modifier
                .padding(vertical = 4.dp)
                .horizontalScroll(rememberScrollState()),
            verticalAlignment = Alignment.CenterVertically
        ) {
            themes.forEach {
                ThemeMiniCard(
                    theme = it,
                    modifier = Modifier.padding(end = 8.dp)
                )
            }
        }

        Spacer(Modifier.height(16.dp))

        AnimatedVisibility(
            visible = selected == null,
            enter = fadeIn() + slideInVertically(initialOffsetY = { it / 2 }),
            exit = fadeOut() + slideOutVertically(targetOffsetY = { -it / 2 })
        ) {
            Column {
                aandachtspunten.forEach { punt ->
                    AttentionCard(
                        title = punt.title,
                        theme = punt.theme,
                        bulletPoints = punt.bulletPoints,
                        onAddBullet = { bullet ->
                            viewModel.addBulletToAandachtspunt(punt.id, bullet)
                        },
                        onUpdateBullets = { updatedBullets ->
                            viewModel.updateAandachtspuntBullets(
                                punt.id,
                                updatedBullets
                            )
                        },
                        onDelete = {
                            viewModel.deleteAandachtspunt(punt.id)
                        },
                        onEdit = { /* handled internally now */ },
                        onDetails = {
                            selectedDetailsItemId = punt.id
                        }
                    )
                }
            }
        }

        AnimatedVisibility(
            visible = selected != null,
            enter = fadeIn() + slideInVertically(initialOffsetY = { it }),
            exit = fadeOut() + slideOutVertically(targetOffsetY = { it })
        ) {
            selected?.let {
                AandachtspuntDetailsSheet(
                    viewModel = viewModel,
                    item = it,
                    onDismiss = { selectedDetailsItemId = null })
            }
        }
    }

    if (showAddSheet) {
        ModalBottomSheet(
            onDismissRequest = { showAddSheet = false }
        ) {
            AddAandachtspuntSheet(
                themes = viewModel.hoofdthemas,
                onDismiss = { showAddSheet = false },
                onCreate = { title, theme ->
                    viewModel.addAandachtspunt(
                        title = title,
                        theme = theme
                    )
                }
            )
        }
    }
}