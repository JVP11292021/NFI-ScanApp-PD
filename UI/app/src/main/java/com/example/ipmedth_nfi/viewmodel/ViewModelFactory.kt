package com.example.ipmedth_nfi.viewmodel

import android.app.Application
import androidx.lifecycle.ViewModel
import androidx.lifecycle.ViewModelProvider
import com.example.ipmedth_nfi.data.export.ProjectStorageManager

class SessionViewModelFactory(
    private val application: Application
) : ViewModelProvider.Factory {

    override fun <T : ViewModel> create(modelClass: Class<T>): T {
        if (modelClass.isAssignableFrom(SessionViewModel::class.java)) {
            val storage = ProjectStorageManager(application)
            @Suppress("UNCHECKED_CAST")
            return SessionViewModel(application, storage) as T
        }
        throw IllegalArgumentException("Unknown ViewModel class")
    }
}