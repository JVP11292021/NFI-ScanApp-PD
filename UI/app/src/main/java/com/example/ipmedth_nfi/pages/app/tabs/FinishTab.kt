package com.example.ipmedth_nfi.pages.app.tabs

import android.widget.Toast
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.Spacer
import androidx.compose.material3.Button
import androidx.compose.material3.Card
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.Checkbox
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.unit.dp
import com.example.ipmedth_nfi.data.export.ProjectExporter
import com.example.ipmedth_nfi.ui.navigation.AssessmentPage
import com.example.ipmedth_nfi.viewmodel.SessionViewModel
import kotlinx.coroutines.launch

@Composable
fun FinishTab(viewModel: SessionViewModel) {
    val pages = AssessmentPage.all.filter { it != AssessmentPage.Finish }
    val incomplete = pages.filter { viewModel.pageCompletion[it] == false }
    val context = LocalContext.current
    val scope = rememberCoroutineScope()

    Box(Modifier.fillMaxSize(), contentAlignment = Alignment.TopCenter) {
        Column(Modifier.fillMaxWidth().padding(16.dp), horizontalAlignment = Alignment.CenterHorizontally) {
            // Warning/info card at top
            Card(
                modifier = Modifier.fillMaxWidth(),
                colors = CardDefaults.cardColors(containerColor = MaterialTheme.colorScheme.secondaryContainer)
            ) {
                Column(Modifier.padding(12.dp)) {
                    if (incomplete.isEmpty()) {
                        Text("Klaar — alle tabbladen zijn ingevuld", style = MaterialTheme.typography.titleMedium)
                        Spacer(modifier = Modifier.padding(6.dp))
                        Text("Je kunt nu het project exporteren of afronden.")
                    } else {
                        Text("Nog te doen", style = MaterialTheme.typography.titleMedium)
                        Spacer(modifier = Modifier.padding(6.dp))
                        Text("De volgende tabbladen moeten nog worden voltooid:")

                        Spacer(modifier = Modifier.padding(8.dp))

                        // List pages with checkboxes to toggle completion
                        Column(Modifier.fillMaxWidth(), verticalArrangement = Arrangement.spacedBy(6.dp)) {
                            pages.forEach { page ->
                                Row(modifier = Modifier.fillMaxWidth(), verticalAlignment = Alignment.CenterVertically, horizontalArrangement = Arrangement.Start) {
                                    val completed = viewModel.pageCompletion[page] == true
                                    Checkbox(
                                        checked = completed,
                                        onCheckedChange = { checked -> viewModel.togglePageCompletion(page, checked) }
                                    )
                                    Spacer(modifier = Modifier.padding(6.dp))
                                    Text(text = page.title)
                                }
                            }
                        }
                    }
                }
            }

            Spacer(modifier = Modifier.padding(12.dp))

            // Export button: ZIP only
            Row(horizontalArrangement = Arrangement.spacedBy(12.dp)) {
                Button(onClick = {
                    scope.launch {
                        val snapshot = viewModel.getCurrentSnapshot()
                        if (snapshot == null) {
                            Toast.makeText(context, "No active onderzoek to export", Toast.LENGTH_LONG).show()
                            return@launch
                        }
                        val file = ProjectExporter.exportProjectZip(context, snapshot)
                        Toast.makeText(context, "Exported ZIP: ${file.absolutePath}", Toast.LENGTH_LONG).show()
                    }
                }) {
                    Text("Export project (ZIP)")
                }
            }

            Spacer(modifier = Modifier.padding(12.dp))
        }
    }
}
