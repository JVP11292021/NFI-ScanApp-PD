package com.example.ipmedth_nfi.ui.components.plan

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Add
import androidx.compose.material.icons.filled.Edit
import androidx.compose.material3.Button
import androidx.compose.material3.Card
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import com.example.ipmedth_nfi.model.Aandachtspunt
import com.example.ipmedth_nfi.model.ActionItem
import com.example.ipmedth_nfi.model.ActionType
import com.example.ipmedth_nfi.ui.icons.TablerCube3dSphere

@Composable
fun PlanCard(
    modifier: Modifier = Modifier,
    item: Aandachtspunt,
    onEditAction: (ActionItem) -> Unit = {},
    onScanSinForAction: (ActionItem) -> Unit = {},
    onNavigateToAnnotation: (String) -> Unit = {}
) {
    Column(modifier = modifier.fillMaxWidth()) {
        // For each primary action, render a small card-like row
        item.primaryActions.forEach { action ->
            // Find matching results in otherActions: simple heuristic match
            val matchingResults = item.otherActions.filter { other ->
                val otherText = listOfNotNull(other.andersBeschrijving, other.beschrijvingEnLocatie).joinToString(" ")
                val actionText = listOfNotNull(action.beschrijvingEnLocatie, action.andersBeschrijving, action.subType?.name).joinToString(" ")
                otherText.contains(actionText, ignoreCase = true) || actionText.contains(otherText, ignoreCase = true)
            }

            Card(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(horizontal = 12.dp, vertical = 8.dp),
                shape = RoundedCornerShape(10.dp)
            ) {
                Row(
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(12.dp),
                    horizontalArrangement = Arrangement.Start
                ) {
                    Column(modifier = Modifier.weight(1f)) {
                        // Beschrijving en locatie
                        Text(text = action.beschrijvingEnLocatie, fontWeight = FontWeight.SemiBold)

                        Spacer(modifier = Modifier.height(6.dp))

                        // Actie(s) line (show subtype if present)
                        val subText = action.subType?.name ?: action.andersBeschrijving
                        Text(text = "Actie(s)", fontWeight = FontWeight.Medium)
                        Spacer(modifier = Modifier.height(4.dp))
                        Text(text = "• ${action.type.name}${if (!subText.isNullOrBlank()) ": $subText" else ""}")

                        Spacer(modifier = Modifier.height(8.dp))

                        // Resultaten - only show when there are matching results
                        if (matchingResults.isNotEmpty()) {
                            Text(text = "Resultaten", fontWeight = FontWeight.Medium)
                            Spacer(modifier = Modifier.height(6.dp))
                            matchingResults.forEach { r ->
                                val resultText = r.andersBeschrijving ?: r.beschrijvingEnLocatie
                                if (resultText.isNotBlank()) {
                                    OutlinedButton(onClick = { /* show result details */ }) {
                                        Text(text = resultText)
                                    }
                                }
                            }
                        }
                    }

                    // Icon area: shown only for Veiligstellen or Bemonsteren
                    val showActionButtons = action.type == ActionType.Veiligstellen || action.type == ActionType.Bemonsteren

                    if (showActionButtons) {
                        Column(modifier = Modifier.padding(start = 8.dp)) {
                            // Edit icon for this action
                            IconButton(onClick = { onEditAction(action) }) {
                                Icon(Icons.Filled.Edit, contentDescription = "Edit Action")
                            }

                            Spacer(modifier = Modifier.height(8.dp))

                            IconButton(onClick = { onNavigateToAnnotation(action.id) }) {
                                Icon(TablerCube3dSphere, contentDescription = "Marker")
                            }

                            Spacer(modifier = Modifier.height(8.dp))

                            // SIN button visible only for these action types
                            Button(onClick = { onScanSinForAction(action) }) {
                                Icon(Icons.Filled.Add, contentDescription = "Scan SIN")
                                Spacer(modifier = Modifier.width(6.dp))
                                Text(text = "Scan SIN")
                            }
                        }
                    }
                }
            }
        }
    }
}
