@echo off
REM ================================
REM COLMAP Sparse Reconstruction BAT
REM ================================

REM ---- EDIT THESE PATHS ----
set COLMAP_EXE=..\bin\Release-windows-x86_64\TestApp\TestApp.exe
set DATASET=pointclouds\well

REM ---- DO NOT EDIT BELOW ----
set IMAGES=%DATASET%\images
set DB=%DATASET%\database.db
set SPARSE=%DATASET%\sparse

echo =================================
echo COLMAP Sparse Reconstruction
echo =================================

REM --- Sanity checks ---
if not exist "%COLMAP_EXE%" (
  echo ERROR: colmap.exe not found at %COLMAP_EXE%
  pause
  exit /b
)

if not exist "%IMAGES%" (
  echo ERROR: images folder not found at %IMAGES%
  pause
  exit /b
)

REM --- Feature extraction ---
echo [1/4] Feature extraction...
"%COLMAP_EXE%" feature_extractor ^
  --database_path "%DB%" ^
  --image_path "%IMAGES%"

if errorlevel 1 goto error

REM --- Feature matching ---
echo [2/4] Feature matching...
"%COLMAP_EXE%" exhaustive_matcher ^
  --database_path "%DB%"

if errorlevel 1 goto error

REM --- Sparse reconstruction ---
echo [3/4] Sparse reconstruction...
if not exist "%SPARSE%" mkdir "%SPARSE%"

"%COLMAP_EXE%" mapper ^
  --database_path "%DB%" ^
  --image_path "%IMAGES%" ^
  --output_path "%SPARSE%"

if errorlevel 1 goto error

REM --- Export sparse model to PLY ---
echo [4/4] Exporting sparse model to PLY...
"%COLMAP_EXE%" model_converter ^
  --input_path "%SPARSE%\0" ^
  --output_path "%SPARSE%\sparse.ply" ^
  --output_type PLY

if errorlevel 1 goto error

echo =================================
echo DONE! Sparse model exported to:
echo %SPARSE%\sparse.ply
echo =================================
pause
exit /b

:error
echo =================================
echo ERROR: COLMAP failed
echo Check the output above for details
echo =================================
pause
exit /b
