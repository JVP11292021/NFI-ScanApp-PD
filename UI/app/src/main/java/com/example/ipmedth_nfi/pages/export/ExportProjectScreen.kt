package com.example.ipmedth_nfi.pages.export

import android.widget.Toast
import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
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
fun ExportProjectScreen(
    viewModel: SessionViewModel,
    onCloseProject: () -> Unit
) {
    val pages = AssessmentPage.entries
    val incomplete = pages.filter { viewModel.pageCompletion[it] == false }
    val context = LocalContext.current
    val scope = rememberCoroutineScope()

    Box(Modifier.fillMaxSize(), contentAlignment = Alignment.TopCenter) {
        Column(
            Modifier
                .fillMaxWidth()
                .padding(16.dp),
            horizontalAlignment = Alignment.CenterHorizontally
        ) {

            Card(
                modifier = Modifier.fillMaxWidth(),
                colors = CardDefaults.cardColors(
                    containerColor = MaterialTheme.colorScheme.secondaryContainer
                )
            ) {
                Column(Modifier.padding(12.dp)) {
                    if (incomplete.isEmpty()) {
                        Text(
                            "Klaar — alle tabbladen zijn ingevuld",
                            style = MaterialTheme.typography.titleMedium
                        )
                        Spacer(Modifier.height(6.dp))
                        Text("Je kunt nu het project exporteren of afsluiten.")
                    } else {
                        Text(
                            "Nog te doen",
                            style = MaterialTheme.typography.titleMedium
                        )
                        Spacer(Modifier.height(6.dp))
                        Text("De volgende tabbladen moeten nog worden voltooid:")

                        Spacer(Modifier.height(8.dp))

                        Column(verticalArrangement = Arrangement.spacedBy(6.dp)) {
                            pages.forEach { page ->
                                Row(
                                    verticalAlignment = Alignment.CenterVertically
                                ) {
                                    Checkbox(
                                        checked = viewModel.pageCompletion[page] == true,
                                        onCheckedChange = { checked ->
                                            viewModel.togglePageCompletion(page, checked)
                                        }
                                    )
                                    Spacer(Modifier.width(6.dp))
                                    Text(page.title)
                                }
                            }
                        }
                    }
                }
            }

            Spacer(Modifier.height(16.dp))

            Button(
                modifier = Modifier.fillMaxWidth(),
                onClick = {
                    scope.launch {
                        val snapshot = viewModel.getCurrentSnapshot()
                        if (snapshot == null) {
                            Toast.makeText(
                                context,
                                "No active onderzoek to export",
                                Toast.LENGTH_LONG
                            ).show()
                            return@launch
                        }

                        val file = ProjectExporter.exportProjectZip(context, snapshot)
                        Toast.makeText(
                            context,
                            "Exported ZIP: ${file.absolutePath}",
                            Toast.LENGTH_LONG
                        ).show()
                    }
                }
            ) {
                Text("Export project (ZIP)")
            }

            Spacer(Modifier.height(8.dp))

            OutlinedButton(
                modifier = Modifier.fillMaxWidth(),
                onClick = {
                    viewModel.closeOnderzoek()
                    onCloseProject()
                }
            ) {
                Text("Save & Close project")
            }
        }
    }
}
