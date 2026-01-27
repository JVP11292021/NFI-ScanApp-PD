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
import androidx.compose.ui.unit.sp
import com.example.ipmedth_nfi.model.Aandachtspunt
import com.example.ipmedth_nfi.model.ActionItem
import com.example.ipmedth_nfi.model.ActionType
import com.example.ipmedth_nfi.ui.icons.TablerCube3dSphere

@Composable
fun PlanCard(
    modifier: Modifier = Modifier,
    item: Aandachtspunt,
    onEditAction: (ActionItem) -> Unit = {},
    onScanSinForAction: (ActionItem) -> Unit = {}
) {
    Column(
        modifier = modifier.fillMaxWidth(),
        verticalArrangement = Arrangement.spacedBy(12.dp)
    ) {
        item.primaryActions.forEach { action ->
            val matchingResults = item.otherActions.filter { other ->
                val otherText = listOfNotNull(other.andersBeschrijving, other.beschrijvingEnLocatie).joinToString(" ")
                val actionText = listOfNotNull(action.beschrijvingEnLocatie, action.andersBeschrijving, action.subType?.name).joinToString(" ")
                otherText.contains(actionText, ignoreCase = true) || actionText.contains(otherText, ignoreCase = true)
            }
            val showActionButtons = action.type == ActionType.Veiligstellen || action.type == ActionType.Bemonsteren

            Card(
                modifier = Modifier
                    .fillMaxWidth(),
                shape = RoundedCornerShape(10.dp)
            ) {
                Row(
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(12.dp),
                    horizontalArrangement = Arrangement.Start
                ) {
                    Column(modifier = Modifier.weight(1f)) {
                        Text("Beschrijving item en locatie", fontSize = 12.sp)
                        Text(text = action.beschrijvingEnLocatie, fontWeight = FontWeight.SemiBold)

                        Spacer(modifier = Modifier.height(6.dp))

                        Text("Actie(s)", fontSize = 12.sp)
                        val subText = action.subType?.name ?: action.andersBeschrijving
                        Spacer(modifier = Modifier.height(4.dp))
                        Text(text = "• ${action.type.name}${if (!subText.isNullOrBlank()) ": $subText" else ""}")

                        Spacer(modifier = Modifier.height(8.dp))

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
                    if (showActionButtons) {
                        Column() {
                            IconButton(onClick = { /* marker action - could open map/marker */ }) {
                                Icon(TablerCube3dSphere, contentDescription = "Marker")
                            }
                        }
                    }
                }
                if (showActionButtons) {
                    Row(
                        modifier = Modifier
                            .fillMaxWidth()
                            .padding(12.dp),
                        horizontalArrangement = Arrangement.Start
                    ) {
                        Button(
                            modifier = Modifier.fillMaxWidth(),
                            onClick = { onScanSinForAction(action) }) {
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
