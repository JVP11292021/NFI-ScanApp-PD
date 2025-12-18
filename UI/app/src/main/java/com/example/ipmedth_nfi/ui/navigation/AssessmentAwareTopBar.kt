package com.example.ipmedth_nfi.ui.navigation

import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.width
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Menu
import androidx.compose.material3.Checkbox
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.Text
import androidx.compose.material3.TopAppBar
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import com.example.ipmedth_nfi.viewmodel.SessionViewModel

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun AssessmentAwareTopBar(
    viewModel: SessionViewModel,
    currentPage: AssessmentPage,
    onMenuClick: () -> Unit
) {
    val isChecked = viewModel.pageCompletion[currentPage] == true
    val enabled = currentPage != AssessmentPage.Finish || viewModel.canCompleteFinish()

    TopAppBar(
        title = {
            Row(verticalAlignment = Alignment.CenterVertically,
                modifier = Modifier.fillMaxWidth()
            ) {
                Checkbox(
                    checked = isChecked,
                    enabled = enabled,
                    onCheckedChange = {
                        viewModel.pageCompletion[currentPage] = it
                    }
                )

                Spacer(Modifier.width(8.dp))

                Text(
                    text = currentPage.title,
                    modifier = Modifier.weight(1f),
                    maxLines = 1
                )

                IconButton(onClick = onMenuClick) {
                    Icon(Icons.Default.Menu, contentDescription = "Menu")
                }
            }
        }
    )
}