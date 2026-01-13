package com.example.ipmedth_nfi.ui.components

import android.os.Build
import androidx.annotation.RequiresApi
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.IntrinsicSize
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxHeight
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Delete
import androidx.compose.material.icons.filled.Favorite
import androidx.compose.material.icons.filled.FavoriteBorder
import androidx.compose.material3.Button
import androidx.compose.material3.Card
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import com.example.ipmedth_nfi.model.Observation
import com.example.ipmedth_nfi.ui.ObservationField
import com.composables.camera

@RequiresApi(Build.VERSION_CODES.O)
@Composable
fun ObservationCard(
    observation: Observation,
    onDelete: () -> Unit,
    onToggleBookmark: () -> Unit,
    onAddImage: () -> Unit
) {
    Card(
        modifier = Modifier.fillMaxWidth(),
        shape = RoundedCornerShape(16.dp)
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(16.dp)
                .height(IntrinsicSize.Max),
            verticalAlignment = Alignment.Top
        ) {

            // LEFT: Main content
            Column(
                modifier = Modifier
                    .weight(1f)
            ) {

                // Timestamp
                ObservationTimestamp(
                    instant = observation.createdAt
                )

                Spacer(Modifier.height(12.dp))

                ObservationField(
                    label = "Beschrijving",
                    value = observation.beschrijving
                )

                ObservationField(
                    label = "Locatie",
                    value = observation.locatie
                )

                ObservationField(
                    label = "Notities",
                    value = observation.notities,
                    large = true
                )

                Spacer(Modifier.height(16.dp))

                Button(
                    onClick = onAddImage,
                    modifier = Modifier.fillMaxWidth()
                ) {
                    Icon(imageVector = camera, contentDescription = "Add Image")
                    Spacer(Modifier.width(4.dp))
                    Text("Foto Toevoegen")
                }
            }

            // RIGHT: Actions column
            Column(
                modifier = Modifier
//                    .padding(start = 8.dp)
                    .fillMaxHeight(),
                horizontalAlignment = Alignment.CenterHorizontally
            ) {
                IconButton(onClick = onDelete) {
                    Icon(
                        imageVector = Icons.Default.Delete,
                        contentDescription = "Delete observation"
                    )
                }

                Spacer(modifier = Modifier.weight(1f))

                IconButton(onClick = onToggleBookmark) {
                    Icon(
                        imageVector =
                            if (observation.isBookmarked)
                                Icons.Default.Favorite
                            else
                                Icons.Default.FavoriteBorder,
                        contentDescription = "Bookmark observation"
                    )
                }
            }
        }
    }
}