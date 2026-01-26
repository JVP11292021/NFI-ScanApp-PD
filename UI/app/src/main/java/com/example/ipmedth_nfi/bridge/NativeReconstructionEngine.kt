package com.example.ipmedth_nfi.bridge
import java.io.File

class NativeReconstructionEngine {
    companion object {
        init {
            System.loadLibrary("renderEngine")
        }
    }

    private external fun nativeCreate(datasetPath: String, databasePath: String);
    private external fun nativeDestroy();
    private external fun nativeExtractMatchFeatures(
        cameraMode: Int = -1,
        descriptorNormalization: String = "l1_root",
        imageListPath: String = ""): Int;
    private external fun nativeReconstruct(
        outputPath: String,
        inputPath: String = "",
        imageListPath: String = "",
        fixExistingFrames: Boolean = true): Int;
    private external fun nativeMapModel(
        inputPath: String,
        outputPath: String,
        outputType: String,
        skipDistortion: Boolean = false): Int;

    private var isInitialized = false;

    fun create(datasetPath: String, databasePath: String) {

        check(!isInitialized) { "Engine already initialized" }
        nativeCreate(datasetPath, databasePath)
        isInitialized = true
    }

    fun destroy() {
        if (isInitialized) {
            nativeDestroy()
            isInitialized = false
        }
    }

    fun extractMatchFeatures(
        cameraMode: Int = -1,
        descriptorNormalization: String = "l1_root",
        imageListPath: String = ""
    ): Int {
        ensureInitialized()
        return nativeExtractMatchFeatures(
            cameraMode,
            descriptorNormalization,
            imageListPath
        )
    }

    fun reconstruct(
        outputPath: String,
        inputPath: String = "",
        imageListPath: String = "",
        fixExistingFrames: Boolean = true
    ): Int {
        ensureInitialized()
        return nativeReconstruct(
            outputPath,
            inputPath,
            imageListPath,
            fixExistingFrames
        )
    }

    fun mapModel(
        inputPath: String,
        outputPath: String,
        outputType: String,
        skipDistortion: Boolean = false
    ): Int {
        ensureInitialized()
        return nativeMapModel(
            inputPath,
            outputPath,
            outputType,
            skipDistortion
        )
    }


    private fun ensureInitialized() {
        check(isInitialized) { "Engine not initialized. Call create() first." }
    }
}