package com.example.ipmedth_nfi.ui.components.buttons

import androidx.compose.material3.Button
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable

@Composable
fun SubheaderButton(title: String, onClick: () -> Unit) {
    Button(onClick = { onClick() }) {
        Text(title)
    }
}