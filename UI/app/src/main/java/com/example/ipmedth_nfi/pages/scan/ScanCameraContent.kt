package com.example.ipmedth_nfi.pages.scan

import android.util.Log
import androidx.camera.core.CameraSelector
import androidx.camera.core.ImageCapture
import androidx.camera.core.ImageCaptureException
import androidx.camera.core.Preview
import androidx.camera.lifecycle.ProcessCameraProvider
import androidx.camera.view.PreviewView
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.FloatingActionButton
import androidx.compose.material3.Scaffold
import androidx.compose.material3.SnackbarDuration
import androidx.compose.material3.SnackbarHost
import androidx.compose.material3.SnackbarHostState
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.DisposableEffect
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.unit.dp
import androidx.compose.ui.viewinterop.AndroidView
import androidx.core.content.ContextCompat
import androidx.core.net.toUri
import androidx.lifecycle.compose.LocalLifecycleOwner
import com.example.ipmedth_nfi.viewmodel.SessionViewModel
import com.example.ipmedth_nfi.bridge.NativeReconstructionEngine
import com.example.ipmedth_nfi.data.export.ProjectStorageManager
import com.example.ipmedth_nfi.exception.ReconstructionException
import com.example.ipmedth_nfi.model.Onderzoek
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.io.File

@Composable
fun ScanCameraContent(
    viewModel: SessionViewModel,
    modifier: Modifier = Modifier
) {
    val context = LocalContext.current
    val lifecycleOwner = LocalLifecycleOwner.current
    val snackbarHostState = remember { SnackbarHostState() }

    var imageCapture by remember { mutableStateOf<ImageCapture?>(null) }
    var isLoading by remember { mutableStateOf(false) }
    var runReconstruction by remember { mutableStateOf(false) }

    val reconstructionEngine = remember {
        NativeReconstructionEngine()
    }

    val activeOnderzoek = viewModel.activeOnderzoek.collectAsState().value
    val projectPath = activeOnderzoek?.let {
        ProjectStorageManager(context).getProjectDir(it)
    }

    // Native engine lifecycle
    DisposableEffect(Unit) {
        val datasetPath = File(projectPath, "/Reconstruction/images")
        val databasePath = File(projectPath, "/Reconstruction/database.db")

        reconstructionEngine.create(
            datasetPath = datasetPath.absolutePath,
            databasePath = databasePath.absolutePath
        )

        onDispose {
            reconstructionEngine.destroy()
        }
    }

    // Snackbar messages
    LaunchedEffect(Unit) {
        viewModel.showNotification.collect { message ->
            snackbarHostState.showSnackbar(message)
        }
    }

    // Reconstruction background job
    LaunchedEffect(runReconstruction) {
        if (!runReconstruction) return@LaunchedEffect

        isLoading = true
        try {
            withContext(Dispatchers.Default) {
                throwIfNotZero(
                reconstructionEngine.extractMatchFeatures())
                throwIfNotZero(
                reconstructionEngine.reconstruct(
                    File(projectPath, "/Reconstruction/sparse").absolutePath),
                    "Reconstructing the image to a 3D environment failed")
                throwIfNotZero(
                    reconstructionEngine.mapModel(
                        File(projectPath, "/Reconstruction/sparse/0").absolutePath,
                        File(projectPath, "/Reconstruction/sparse/sparse.ply").absolutePath,
                        "PLY"), "Was not able to export sparse model to PLY.")

            }
        }
        catch (e: ReconstructionException) {
            snackbarHostState.showSnackbar(
                message = e.toString(),
                duration = SnackbarDuration.Short
            )
        }
        finally {
            isLoading = false
            runReconstruction = false
        }
    }

    Scaffold(
        snackbarHost = { SnackbarHost(snackbarHostState) }
    ) { padding ->

        Box(
            modifier = modifier
                .fillMaxSize()
                .padding(padding)
        ) {

            // Camera preview
            AndroidView(
                modifier = Modifier.fillMaxSize(),
                factory = { ctx ->
                    val previewView = PreviewView(ctx)
                    val cameraProviderFuture =
                        ProcessCameraProvider.getInstance(ctx)

                    cameraProviderFuture.addListener({
                        val cameraProvider = cameraProviderFuture.get()

                        val preview = Preview.Builder().build().also {
                            it.surfaceProvider = previewView.surfaceProvider
                        }

                        imageCapture = ImageCapture.Builder()
                            .setCaptureMode(ImageCapture.CAPTURE_MODE_MINIMIZE_LATENCY)
                            .build()

                        cameraProvider.unbindAll()
                        cameraProvider.bindToLifecycle(
                            lifecycleOwner,
                            CameraSelector.DEFAULT_BACK_CAMERA,
                            preview,
                            imageCapture
                        )
                    }, ContextCompat.getMainExecutor(ctx))

                    previewView
                }
            )

            FloatingActionButton(
                modifier = Modifier
                    .align(Alignment.BottomCenter)
                    .padding(bottom = 64.dp),
                onClick = {
                    if (!isLoading) {
                        imageCapture?.let {
                            takePhoto(it, viewModel)
                        }
                    }
                }
            ) {
                Text("📸")
            }

            FloatingActionButton(
                modifier = Modifier
                    .align(Alignment.BottomEnd)
                    .padding(bottom = 64.dp, end = 24.dp),
                onClick = {
                    if (!isLoading) {
                        runReconstruction = true
                    }
                }
            ) {
                Text("⚙️")
            }

            // 🔄 Loading overlay
            if (isLoading) {
                Box(
                    modifier = Modifier
                        .fillMaxSize()
                        .background(Color.Black.copy(alpha = 0.4f)),
                    contentAlignment = Alignment.Center
                ) {
                    Column(horizontalAlignment = Alignment.CenterHorizontally) {
                        CircularProgressIndicator()
                        Spacer(Modifier.height(16.dp))
                        Text("Processing…", color = Color.White)
                    }
                }
            }
        }
    }
}

private fun takePhoto(
    imageCapture: ImageCapture,
    viewModel: SessionViewModel
) {
    val photoFile = viewModel.createImageFile()

    val outputOptions = ImageCapture.OutputFileOptions
        .Builder(photoFile)
        .build()

    imageCapture.takePicture(
        outputOptions,
        ContextCompat.getMainExecutor(viewModel.getApplication()),
        object : ImageCapture.OnImageSavedCallback {

            override fun onImageSaved(
                outputFileResults: ImageCapture.OutputFileResults
            ) {
                viewModel.onPhotoCaptured(photoFile.toUri())
            }

            override fun onError(exception: ImageCaptureException) {
                exception.printStackTrace()
            }
        }
    )
}

private fun throwIfNotZero(resultCode: Int, message: String = "") {
    if (resultCode != 0) {
        throw ReconstructionException("Native engine error: $resultCode, $message")
    }
}