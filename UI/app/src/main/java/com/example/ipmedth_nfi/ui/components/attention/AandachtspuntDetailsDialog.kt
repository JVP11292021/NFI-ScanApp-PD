package com.example.ipmedth_nfi.ui.components.attention

import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.outlined.Close
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.geometry.Size
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.dp
import com.example.ipmedth_nfi.model.*
import com.example.ipmedth_nfi.ui.components.common.EditableRow
import com.example.ipmedth_nfi.viewmodel.SessionViewModel
import kotlin.random.Random
import java.util.UUID
import androidx.core.graphics.toColorInt

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun AandachtspuntDetailsSheet(
    viewModel: SessionViewModel,
    item: Aandachtspunt,
    onDismiss: () -> Unit
) {
    val relevanteScenes = remember { mutableStateListOf<String>().apply { addAll(item.relevanteScenes) } }
    val probabilities = remember { mutableStateListOf<SceneProbability>().apply { addAll(item.sceneProbabilities) } }
    val verwachte = remember {
        mutableStateListOf<Pair<String, MutableList<String>>>()
            .apply { item.verwachteSporen.forEach { add(it.key to it.value.toMutableList()) } }
    }
    val primaryActions = remember { mutableStateListOf<ActionItem>().apply { addAll(item.primaryActions) } }
    val otherActions = remember { mutableStateListOf<ActionItem>().apply { addAll(item.otherActions) } }

    var newSceneText by remember { mutableStateOf("") }
    val scroll = rememberScrollState()

    // Editing states
    var editingSceneIndex by remember { mutableStateOf<Int?>(null) }
    var editingSceneText by remember { mutableStateOf("") }

    var editingSpoorSceneIndex by remember { mutableStateOf<Int?>(null) }
    var editingSpoorIndex by remember { mutableStateOf<Int?>(null) }
    var editingSpoorText by remember { mutableStateOf("") }

    var editingActionIsPrimary by remember { mutableStateOf(true) }
    var editingActionIndex by remember { mutableStateOf<Int?>(null) }
    var editingActionItem by remember { mutableStateOf<ActionItem?>(null) }

    // palette for color selection
    val palette = listOf(0xFFE57373.toInt(), 0xFFFFB74D.toInt(), 0xFFFFF176.toInt(), 0xFF81C784.toInt(), 0xFF64B5F6.toInt(), 0xFFBA68C8.toInt())

    fun cycleColor(currentHex: String): String {
        val cur = runCatching { currentHex.toColorInt() }.getOrDefault(palette[0])
        val idx = palette.indexOfFirst { it == cur }
        val next = if (idx == -1 || idx == palette.lastIndex) palette[0] else palette[idx + 1]
        return String.format("#%06X", 0xFFFFFF and next)
    }

    Column(
        modifier = Modifier
            .fillMaxWidth()
            .fillMaxHeight(0.92f)
            .verticalScroll(scroll)
            .padding(12.dp)
    ) {

        /* ---------- HEADER ---------- */

        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.SpaceBetween,
            verticalAlignment = Alignment.CenterVertically
        ) {
            Text("Details", style = MaterialTheme.typography.titleLarge)
            IconButton(onClick = onDismiss) { Icon(Icons.Outlined.Close, contentDescription = "Close") }
        }

        Spacer(Modifier.height(10.dp))

        /* ---------- AANDACHTSPUNT ---------- */

        Box(
            modifier = Modifier
                .clip(RoundedCornerShape(8.dp))
                .background(MaterialTheme.colorScheme.primaryContainer)
                //.border(1.dp, Color.Blue, RoundedCornerShape(8.dp))
                .fillMaxWidth()
                .padding(12.dp)
        ) {
            Column {
                Text("Aandachtspunt", style = MaterialTheme.typography.titleMedium)
                Text(item.title, color = MaterialTheme.colorScheme.onPrimaryContainer)
            }
        }

        Spacer(Modifier.height(8.dp))

        /* ---------- THEME ---------- */

        Box(
            modifier = Modifier
                .clip(RoundedCornerShape(8.dp))
                .background(MaterialTheme.colorScheme.secondaryContainer)
                .fillMaxWidth()
                .padding(12.dp)
        ) {
            Column {
                Text("Hoofdthema", style = MaterialTheme.typography.titleMedium)
                Text(item.theme.name, color = MaterialTheme.colorScheme.onSecondaryContainer)
            }
        }

        Spacer(Modifier.height(12.dp))

        /* ---------- RELEVANTE SCENES ---------- */

        SectionTitle("Relevante scenes")

        relevanteScenes.toList().forEachIndexed { idx, scene ->
            EditableRow(
                text = scene,
                onEdit = {
                    editingSceneIndex = idx
                    editingSceneText = scene
                },
                onDelete = { relevanteScenes.remove(scene); probabilities.removeAll { it.scene == scene }; verwachte.removeAll { it.first == scene } }
            )
        }

        OutlinedTextField(
            value = newSceneText,
            onValueChange = { newSceneText = it },
            label = { Text("Nieuwe scene") },
            modifier = Modifier.fillMaxWidth()
        )

        Spacer(Modifier.height(6.dp))

        Button(onClick = {
            if (newSceneText.isNotBlank()) {
                val t = newSceneText.trim()
                relevanteScenes.add(t)
                probabilities.add(SceneProbability(t, 0, randomColorHex()))
                verwachte.add(t to mutableListOf())
                newSceneText = ""
            }
        }) { Text("Toevoegen") }

        Spacer(Modifier.height(12.dp))

        /* ---------- WAARSCHIJNLIJKHEID (SLIDERS + COLORS) ---------- */

        SectionTitle("Waarschijnlijkheid")

        probabilities.forEachIndexed { idx, p ->
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(vertical = 6.dp),
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text(p.scene, modifier = Modifier.weight(1f))
                // color box
                Box(modifier = Modifier
                    .size(28.dp)
                    .background(Color(p.colorHex.toColorInt()))
                    .border(1.dp, Color.Black, RoundedCornerShape(4.dp))
                    .padding(4.dp)
                    .clickable {
                        probabilities[idx] = p.copy(colorHex = cycleColor(p.colorHex))
                    }
                ) {}

                Spacer(Modifier.width(8.dp))

                Slider(
                    value = p.percent / 100f,
                    onValueChange = {
                        probabilities[idx] = p.copy(percent = (it * 100).toInt())
                    },
                    modifier = Modifier.weight(1f)
                )

                Spacer(Modifier.width(8.dp))
                Text("${p.percent}%")
            }
        }

        Spacer(Modifier.height(6.dp))

        Button(onClick = {
            if (probabilities.isNotEmpty()) {
                val equal = 100 / probabilities.size
                probabilities.forEachIndexed { i, old -> probabilities[i] = old.copy(percent = equal) }
                val diff = 100 - probabilities.sumOf { it.percent }
                probabilities[probabilities.lastIndex] = probabilities.last().copy(percent = probabilities.last().percent + diff)
            }
        }) { Text("Verdeel gelijk") }

        Spacer(Modifier.height(12.dp))

        // Live PieChart replaced with Compose Canvas implementation
        PieChartCompose(probabilities = probabilities)

        Spacer(Modifier.height(12.dp))

        /* ---------- VERWACHT SPOREN ---------- */

        SectionTitle("Verwacht sporenbeeld")

        verwachte.forEachIndexed { sceneIdx, pair ->
            Column(modifier = Modifier.fillMaxWidth().padding(vertical = 6.dp)) {
                Text(pair.first, style = MaterialTheme.typography.titleSmall)

                pair.second.toList().forEachIndexed { idx, spoor ->
                    EditableRow(
                        text = spoor,
                        onEdit = {
                            editingSpoorSceneIndex = sceneIdx
                            editingSpoorIndex = idx
                            editingSpoorText = spoor
                        },
                        onDelete = { pair.second.remove(spoor) }
                    )
                }

                Spacer(Modifier.height(6.dp))
                var newSpoor by remember { mutableStateOf("") }
                OutlinedTextField(value = newSpoor, onValueChange = { newSpoor = it }, label = { Text("Nieuw spoor") }, modifier = Modifier.fillMaxWidth())
                Spacer(Modifier.height(6.dp))
                Button(onClick = {
                    if (newSpoor.isNotBlank()) {
                        pair.second.add(newSpoor.trim())
                        newSpoor = ""
                    }
                }) { Text("Toevoegen") }
            }
        }

        Spacer(Modifier.height(12.dp))

        /* ---------- ACTIES ---------- */

        SectionTitle("Te ondernemen acties")

        // Provide a small inline form to add primary actions with proper type/subtype handling
        var newPrimaryBesch by remember { mutableStateOf("") }
        var newPrimaryType by remember { mutableStateOf(ActionType.Veiligstellen) }
        var newPrimarySub by remember { mutableStateOf(SubActionType.Epitheel) }
        var newPrimaryAnders by remember { mutableStateOf("") }

        Column(modifier = Modifier.fillMaxWidth().padding(vertical = 6.dp)) {
            OutlinedTextField(value = newPrimaryBesch, onValueChange = { newPrimaryBesch = it }, label = { Text("Beschrijving en locatie") }, modifier = Modifier.fillMaxWidth())

            Spacer(Modifier.height(8.dp))

            // ActionType dropdown
            var expandedNewType by remember { mutableStateOf(false) }
            ExposedDropdownMenuBox(expanded = expandedNewType, onExpandedChange = { expandedNewType = !expandedNewType }) {
                OutlinedTextField(
                    readOnly = true,
                    value = newPrimaryType.name,
                    onValueChange = {},
                    label = { Text("Actie") },
                    trailingIcon = { ExposedDropdownMenuDefaults.TrailingIcon(expandedNewType) },
                    modifier = Modifier.menuAnchor()
                )
                ExposedDropdownMenu(expanded = expandedNewType, onDismissRequest = { expandedNewType = false }) {
                    ActionType.entries.forEach { t ->
                        DropdownMenuItem(text = { Text(t.name) }, onClick = { newPrimaryType = t; expandedNewType = false })
                    }
                }
            }

            Spacer(Modifier.height(8.dp))

            // Show subaction selector when Bemonsteren, or Anders text input when top-level Anders
            if (newPrimaryType == ActionType.Bemonsteren) {
                 var expandedNewSub by remember { mutableStateOf(false) }
                 ExposedDropdownMenuBox(expanded = expandedNewSub, onExpandedChange = { expandedNewSub = !expandedNewSub }) {
                     OutlinedTextField(readOnly = true, value = newPrimarySub.name, onValueChange = {}, label = { Text("Subactie (monster) ") }, trailingIcon = { ExposedDropdownMenuDefaults.TrailingIcon(expandedNewSub) }, modifier = Modifier.menuAnchor())
                     ExposedDropdownMenu(expanded = expandedNewSub, onDismissRequest = { expandedNewSub = false }) {
                         SubActionType.entries.forEach { s -> DropdownMenuItem(text = { Text(s.name) }, onClick = { newPrimarySub = s; expandedNewSub = false }) }
                     }
                 }

                 if (newPrimarySub == SubActionType.Anders) {
                     Spacer(Modifier.height(8.dp))
                     OutlinedTextField(value = newPrimaryAnders, onValueChange = { newPrimaryAnders = it }, label = { Text("Anders (beschrijving)") }, modifier = Modifier.fillMaxWidth())
                 }
            } else if (newPrimaryType == ActionType.Anders) {
                Spacer(Modifier.height(8.dp))
                OutlinedTextField(value = newPrimaryAnders, onValueChange = { newPrimaryAnders = it }, label = { Text("Anders (beschrijving)") }, modifier = Modifier.fillMaxWidth())
            }

            Spacer(Modifier.height(8.dp))
            Row {
                Button(onClick = {
                    if (newPrimaryBesch.isNotBlank()) {
                        val itemToAdd = ActionItem(
                            id = UUID.randomUUID().toString(),
                            beschrijvingEnLocatie = newPrimaryBesch.trim(),
                            type = newPrimaryType,
                            subType = if (newPrimaryType == ActionType.Bemonsteren) newPrimarySub else null,
                            andersBeschrijving = when {
                                newPrimaryType == ActionType.Anders -> newPrimaryAnders.ifBlank { null }
                                newPrimaryType == ActionType.Bemonsteren && newPrimarySub == SubActionType.Anders -> newPrimaryAnders.ifBlank { null }
                                else -> null
                            }
                        )
                        primaryActions.add(itemToAdd)
                        // reset
                        newPrimaryBesch = ""
                        newPrimaryType = ActionType.Veiligstellen
                        newPrimarySub = SubActionType.Epitheel
                        newPrimaryAnders = ""
                    }
                }) { Text("Voeg toe") }

                Spacer(Modifier.width(8.dp))
                TextButton(onClick = {
                    // quick add a Veiligstellen if user prefers
                    val quick = ActionItem(UUID.randomUUID().toString(), "Veiligstellen", ActionType.Veiligstellen)
                    primaryActions.add(quick)
                }) { Text("Snel: Veiligstellen") }
            }
        }

        Spacer(Modifier.height(10.dp))

        primaryActions.toList().forEachIndexed { idx, act ->
            EditableRow(
                text = act.beschrijvingEnLocatie + " (" + act.type.name + (act.subType?.let { ", ${it.name}" } ?: "") + ")",
                onEdit = {
                    editingActionIsPrimary = true
                    editingActionIndex = idx
                    editingActionItem = act
                },
                onDelete = { primaryActions.remove(act) }
            )
        }

        Button(onClick = {
            // keep compatibility: previous behavior added a default Veiligstellen
            primaryActions.add(ActionItem(UUID.randomUUID().toString(), "Beschrijving/locatie", ActionType.Veiligstellen))
        }) { Text("Actie toevoegen") }

        Spacer(Modifier.height(10.dp))

        SectionTitle("Overige acties")

        // Inline add for otherActions (top-level Anders)
        var newOtherText by remember { mutableStateOf("") }
        OutlinedTextField(value = newOtherText, onValueChange = { newOtherText = it }, label = { Text("Beschrijving en locatie (Anders)") }, modifier = Modifier.fillMaxWidth())
        Spacer(Modifier.height(8.dp))
        Row {
            Button(onClick = {
                if (newOtherText.isNotBlank()) {
                    otherActions.add(ActionItem(UUID.randomUUID().toString(), newOtherText.trim(), ActionType.Anders, subType = null, andersBeschrijving = newOtherText.trim()))
                    newOtherText = ""
                }
            }) { Text("Voeg toe") }
            Spacer(Modifier.width(8.dp))
            TextButton(onClick = { otherActions.add(ActionItem(UUID.randomUUID().toString(), "Anders", ActionType.Anders, subType = null, andersBeschrijving = "Anders")) }) { Text("Snel: Anders") }
        }

        Spacer(Modifier.height(10.dp))

        otherActions.toList().forEachIndexed { idx, act ->
            EditableRow(
                text = act.beschrijvingEnLocatie + " (" + act.type.name + ")",
                onEdit = {
                    editingActionIsPrimary = false
                    editingActionIndex = idx
                    editingActionItem = act
                },
                onDelete = { otherActions.remove(act) }
            )
        }

        Spacer(Modifier.height(16.dp))

        /* ---------- SAVE ---------- */

        Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.End) {
            TextButton(onClick = onDismiss) { Text("Annuleren") }
            Spacer(Modifier.width(8.dp))
            Button(onClick = {
                viewModel.updateAandachtspunt(
                    item.copy(
                        relevanteScenes = relevanteScenes.toList(),
                        sceneProbabilities = probabilities.toList(),
                        verwachteSporen = verwachte.associate { it.first to it.second.toList() },
                        primaryActions = primaryActions.toList(),
                        otherActions = otherActions.toList()
                    )
                )
                onDismiss()
            }) { Text("Opslaan") }
        }
    }

    // ---------- EDIT DIALOGS ----------

    if (editingSceneIndex != null) {
        AlertDialog(
            onDismissRequest = { editingSceneIndex = null },
            title = { Text("Bewerk scene") },
            text = {
                OutlinedTextField(value = editingSceneText, onValueChange = { editingSceneText = it }, label = { Text("Scene") }, modifier = Modifier.fillMaxWidth())
            },
            confirmButton = {
                TextButton(onClick = {
                    val idx = editingSceneIndex ?: return@TextButton
                    val old = relevanteScenes[idx]
                    relevanteScenes[idx] = editingSceneText.trim()
                    // update related probabilities and expected spoor keys
                    probabilities.indexOfFirst { it.scene == old }.takeIf { it != -1 }?.let { pi -> probabilities[pi] = probabilities[pi].copy(scene = editingSceneText.trim()) }
                    verwachte.indexOfFirst { it.first == old }.takeIf { it != -1 }?.let { wi -> verwachte[wi] = editingSceneText.trim() to verwachte[wi].second }
                    editingSceneIndex = null
                }) { Text("Opslaan") }
            },
            dismissButton = { TextButton(onClick = { editingSceneIndex = null }) { Text("Annuleren") } }
        )
    }

    if (editingSpoorSceneIndex != null && editingSpoorIndex != null) {
        AlertDialog(
            onDismissRequest = { editingSpoorSceneIndex = null; editingSpoorIndex = null },
            title = { Text("Bewerk spoor") },
            text = {
                OutlinedTextField(value = editingSpoorText, onValueChange = { editingSpoorText = it }, label = { Text("Spoor") }, modifier = Modifier.fillMaxWidth())
            },
            confirmButton = {
                TextButton(onClick = {
                    val sIdx = editingSpoorSceneIndex ?: return@TextButton
                    val spIdx = editingSpoorIndex ?: return@TextButton
                    verwachte[sIdx].second[spIdx] = editingSpoorText.trim()
                    editingSpoorSceneIndex = null
                    editingSpoorIndex = null
                }) { Text("Opslaan") }
            },
            dismissButton = { TextButton(onClick = { editingSpoorSceneIndex = null; editingSpoorIndex = null }) { Text("Annuleren") } }
        )
    }

    if (editingActionIndex != null && editingActionItem != null) {
        // action edit dialog
        var tmpBeschrijving by remember { mutableStateOf(editingActionItem!!.beschrijvingEnLocatie) }
        var tmpType by remember { mutableStateOf(editingActionItem!!.type) }
        var tmpSub by remember { mutableStateOf(editingActionItem!!.subType ?: SubActionType.Anders) }
        var tmpAnders by remember { mutableStateOf(editingActionItem!!.andersBeschrijving ?: "") }

        AlertDialog(
            onDismissRequest = { editingActionIndex = null; editingActionItem = null },
            title = { Text("Bewerk actie") },
            text = {
                Column {
                    OutlinedTextField(value = tmpBeschrijving, onValueChange = { tmpBeschrijving = it }, label = { Text("Beschrijving en locatie") })

                    Spacer(Modifier.height(8.dp))

                    // simple dropdown for ActionType
                    var expandedType by remember { mutableStateOf(false) }
                    ExposedDropdownMenuBox(expanded = expandedType, onExpandedChange = { expandedType = !expandedType }) {
                        OutlinedTextField(
                            readOnly = true,
                            value = tmpType.name,
                            onValueChange = {},
                            label = { Text("Actie") },
                            trailingIcon = { ExposedDropdownMenuDefaults.TrailingIcon(expandedType) },
                            modifier = Modifier.menuAnchor()
                        )
                        ExposedDropdownMenu(expanded = expandedType, onDismissRequest = { expandedType = false }) {
                            ActionType.entries.forEach { t ->
                                DropdownMenuItem(text = { Text(t.name) }, onClick = { tmpType = t; expandedType = false })
                            }
                        }
                    }

                    Spacer(Modifier.height(8.dp))

                    // show subaction selector only when Bemonsteren is selected
                    if (tmpType == ActionType.Bemonsteren) {
                         var expandedSub by remember { mutableStateOf(false) }
                         ExposedDropdownMenuBox(expanded = expandedSub, onExpandedChange = { expandedSub = !expandedSub }) {
                             OutlinedTextField(readOnly = true, value = tmpSub.name, onValueChange = {}, label = { Text("Subactie") }, trailingIcon = { ExposedDropdownMenuDefaults.TrailingIcon(expandedSub) }, modifier = Modifier.menuAnchor())
                             ExposedDropdownMenu(expanded = expandedSub, onDismissRequest = { expandedSub = false }) {
                                 SubActionType.entries.forEach { s -> DropdownMenuItem(text = { Text(s.name) }, onClick = { tmpSub = s; expandedSub = false }) }
                             }
                         }

                         if (tmpSub == SubActionType.Anders) {
                             Spacer(Modifier.height(8.dp))
                             OutlinedTextField(value = tmpAnders, onValueChange = { tmpAnders = it }, label = { Text("Anders (beschrijving)") })
                         }
                     } else if (tmpType == ActionType.Anders) {
                        // When editing a top-level Anders action, allow a separate Anders description input
                        Spacer(Modifier.height(8.dp))
                        OutlinedTextField(value = tmpAnders, onValueChange = { tmpAnders = it }, label = { Text("Anders (beschrijving)") })
                     }
                 }
             },
             confirmButton = {
                 TextButton(onClick = {
                     val idx = editingActionIndex ?: return@TextButton
                     val newSub = if (tmpType == ActionType.Bemonsteren) tmpSub else null
                     val newAnders = when {
                         tmpType == ActionType.Anders -> tmpAnders.ifBlank { null }
                         tmpType == ActionType.Bemonsteren && tmpSub == SubActionType.Anders -> tmpAnders.ifBlank { null }
                         else -> null
                     }
                     val newItem = editingActionItem!!.copy(beschrijvingEnLocatie = tmpBeschrijving, type = tmpType, subType = newSub, andersBeschrijving = newAnders)
                     if (editingActionIsPrimary) {
                         primaryActions[idx] = newItem
                     } else {
                         otherActions[idx] = newItem
                     }
                     editingActionIndex = null
                     editingActionItem = null
                 }) { Text("Opslaan") }
             },
             dismissButton = { TextButton(onClick = { editingActionIndex = null; editingActionItem = null }) { Text("Annuleren") } }
         )
     }
 }

/* ---------- SMALL REUSABLE UI ---------- */

@Composable
private fun SectionTitle(text: String) {
    Text(text, style = MaterialTheme.typography.titleMedium)
}

@Composable
private fun PieChartCompose(probabilities: List<SceneProbability>, sizeDp: Dp = 200.dp) {
    val total = probabilities.sumOf { it.percent }
    // draw with Canvas
    androidx.compose.foundation.Canvas(modifier = Modifier
        .fillMaxWidth()
        .height(sizeDp)
    ) {
        var startAngle = -90f
        val cx = size.width / 2
        val cy = size.height / 2
        val radius = (size.minDimension / 2) * 0.9f
        probabilities.forEach { sp ->
            val angle = if (total == 0) 0f else (sp.percent.toFloat() / total.toFloat()) * 360f
            drawArc(
                color = Color(sp.colorHex.toColorInt()),
                startAngle = startAngle,
                sweepAngle = angle,
                useCenter = true,
                topLeft = androidx.compose.ui.geometry.Offset(cx - radius, cy - radius),
                size = Size(radius * 2, radius * 2)
            )
            startAngle += angle
        }
    }
}

private fun randomColorHex(): String {
    return String.format("#%02X%02X%02X",
        Random.nextInt(30, 230),
        Random.nextInt(30, 230),
        Random.nextInt(30, 230)
    )
}
