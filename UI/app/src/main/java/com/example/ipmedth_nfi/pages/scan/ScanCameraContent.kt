package com.example.ipmedth_nfi.pages.scan

import android.util.Log
import androidx.camera.core.CameraSelector
import androidx.camera.core.ImageCapture
import androidx.camera.core.ImageCaptureException
import androidx.camera.core.Preview
import androidx.camera.lifecycle.ProcessCameraProvider
import androidx.camera.view.PreviewView
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.FloatingActionButton
import androidx.compose.material3.Scaffold
import androidx.compose.material3.SnackbarDuration
import androidx.compose.material3.SnackbarHost
import androidx.compose.material3.SnackbarHostState
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.DisposableEffect
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.unit.dp
import androidx.compose.ui.viewinterop.AndroidView
import androidx.core.content.ContextCompat
import androidx.core.net.toUri
import androidx.lifecycle.compose.LocalLifecycleOwner
import com.example.ipmedth_nfi.viewmodel.SessionViewModel
import com.example.ipmedth_nfi.bridge.NativeReconstructionEngine
import com.example.ipmedth_nfi.data.export.ProjectStorageManager
import com.example.ipmedth_nfi.model.Onderzoek

@Composable
fun ScanCameraContent(
    viewModel: SessionViewModel,
    modifier: Modifier = Modifier
) {
    val context = LocalContext.current
    val lifecycleOwner = LocalLifecycleOwner.current
    val snackbarHostState = remember { SnackbarHostState() }
    var imageCapture by remember { mutableStateOf<ImageCapture?>(null) }

    val reconstructionEngine = remember {
        NativeReconstructionEngine()
    }

    // TODO make this dynamic
    DisposableEffect(Unit) {
        val datasetPath = "/storage/emulated/0/Android/data/com.example.ipmedth_nfi/files/NFI_Scanapp/dgbbnj/testCase/Reconstruction/images"
        val databasePath = "/storage/emulated/0/Android/data/com.example.ipmedth_nfi/files/NFI_Scanapp/dgbbnj/testCase/Reconstruction"

        reconstructionEngine.create(
            datasetPath = datasetPath,
            databasePath = databasePath
        )

        onDispose {
            reconstructionEngine.destroy()
        }
    }

    LaunchedEffect(Unit) {
        viewModel.showNotification.collect { message ->
            snackbarHostState.showSnackbar(
                message = message,
                duration = SnackbarDuration.Short
            )
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
                    imageCapture?.let {
                        takePhoto(it, viewModel)
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
                    Log.i("NFI", "On Sfm pipeline");
                    var resultCode = reconstructionEngine.extractMatchFeatures()
//                    resultCode = reconstructionEngine.reconstruct()

                }
            ) {
                Text("⚙️")
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
