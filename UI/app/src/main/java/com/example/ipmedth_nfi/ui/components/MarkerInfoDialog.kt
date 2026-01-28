package com.example.ipmedth_nfi.ui.components

import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.height
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.Button
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import com.example.ipmedth_nfi.model.ActionItem

@Composable
fun MarkerInfoDialog(
    action: ActionItem,
    onDismiss: () -> Unit,
    coordinates: FloatArray? = null
) {
    AlertDialog(
        onDismissRequest = onDismiss,
        title = { Text("Actie Informatie") },
        text = {
            Column {
                Text(
                    text = "Beschrijving en Locatie:",
                    style = MaterialTheme.typography.labelMedium,
                    fontWeight = FontWeight.Bold
                )
                Text(text = action.beschrijvingEnLocatie)

                Spacer(modifier = Modifier.height(8.dp))

                Text(
                    text = "Type:",
                    style = MaterialTheme.typography.labelMedium,
                    fontWeight = FontWeight.Bold
                )
                Text(text = action.type.name)

                action.subType?.let { subType ->
                    Spacer(modifier = Modifier.height(8.dp))
                    Text(
                        text = "Subtype:",
                        style = MaterialTheme.typography.labelMedium,
                        fontWeight = FontWeight.Bold
                    )
                    Text(text = subType.name)
                }

                action.andersBeschrijving?.let { anders ->
                    Spacer(modifier = Modifier.height(8.dp))
                    Text(
                        text = "Anders:",
                        style = MaterialTheme.typography.labelMedium,
                        fontWeight = FontWeight.Bold
                    )
                    Text(text = anders)
                }

                coordinates?.let { coords ->
                    if (coords.size == 3) {
                        Spacer(modifier = Modifier.height(8.dp))
                        Text(
                            text = "Coördinaten:",
                            style = MaterialTheme.typography.labelMedium,
                            fontWeight = FontWeight.Bold
                        )
                        Text(text = "X: %.3f".format(coords[0]))
                        Text(text = "Y: %.3f".format(coords[1]))
                        Text(text = "Z: %.3f".format(coords[2]))
                    }
                }
            }
        },
        confirmButton = {
            Button(onClick = onDismiss) {
                Text("Sluiten")
            }
        }
    )
}
