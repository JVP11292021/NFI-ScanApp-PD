# Scan App PD

This application is designed to support forensic investigations by transforming photographs into structured, annotated records. Users can capture detailed images of objects, rooms, or entire scenes and place precise annotations directly onto the photos to document observations, evidence, and contextual details.

Each annotation is anchored to a specific location within the image, helping investigators clearly record measurements, damage, points of interest, and investigative notes without ambiguity. The result is a clear, visual reference that preserves spatial relationships and reduces reliance on memory or separate written notes.

Built for accuracy, clarity, and traceability, the application streamlines documentation workflows while maintaining the integrity of forensic data. It serves as a reliable tool for investigators who need to capture, analyze, and communicate findings with confidence.


## Build

This application is built in specific for the **Android** platform. The project contains native `C++` and `Kotlin` components. To learn more about the code components please see the "More Info" section for further reference.

This project was build and tested with the following tools and versions:
1. Android studio - 2025.2.2 Patch 1
2. CMake - V3.22.1
3. Gradle - V8.13
4. Git LFS

### Compile Targets

The project is compiled with the following properties:
- compileSDK = **36**
- minSDK = **26**
- NDK = **arm64-v8a**

## Installation
### Getting the Source Code
You can start with the latest stable release . Or if you want the latest version, you can clone the git repository
```BASH
https://github.com/JVP11292021/NFI-ScanApp-PD.git
```

### Dependencies

> [!NOTE]
> Scan App PD  requires a **fully C++17-compliant** compiler.

Scan App PD relies on a number of open source libraries, none of which are optional. The native `C++` component has the following third party dependencies:
- [Ceres Solver - V2.1](http://ceres-solver.org/index.html)
- [Assimp - V6.0.2](https://assimp.org/)
- [Eigen - V3.3.4](https://github.com/PX4/eigen)
- [CLAPACK - Vx.x](https://github.com/alphacep/clapack/tree/master/SRC)
- [FAISS - Custom built Android version](https://faiss.ai/index.html)
- [FreeImage - V3.19.10](https://freeimage.sourceforge.io/)
- [GLM - V1.0.3](https://github.com/g-truc/glm)
- [PoseLib - V2.0.5](https://github.com/PoseLib/PoseLib)
- [sqlite3 - V3.51.1](https://github.com/SRombauts/SQLiteCpp)
- [tinyobjloader - V1.0.6](https://github.com/tinyobjloader/tinyobjloader)
- [VLFeat - V0.9.21](https://www.vlfeat.org/)
- [VulkanLiteEngine - V1.0.0](https://github.com/JVP11292021/VulkanLiteEngine)

> [!NOTE]
> The listed dependencies are already embedded in the project.

### Platform Building
-   Open the project in **Android Studio**
-   Allow Gradle to sync dependencies
-   Select a target device or emulator
-   Build the project:
    -   **Menu:** `Build → Make Project`
    -   or via terminal:
        ```BASH
        ./gradlew assembleDebug`
        ```

## More Info
To learn more about tis project you can take a look at the project wiki. Inside the wiki the `minmap` and `VulkanLiteEngine` dependencies will be explained in greater detail.