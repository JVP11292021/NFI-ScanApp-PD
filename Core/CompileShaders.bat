@echo off
setlocal enabledelayedexpansion

if not defined VULKAN_SDK (
    echo ERROR: Vulkan SDK environment variable not set!
    pause
    exit /b 1
)

echo Using Vulkan SDK at: %VULKAN_SDK%
echo Searching project for shader files...
echo.

for /r %%F in (*.vert *.frag) do (
    set "INFILE=%%F"
    set "OUTFILE=%%F.spv"

    echo Compiling: %%F

    "%VULKAN_SDK%\Bin\glslc.exe" "%%F" -o "!OUTFILE!" || (
        echo ERROR: Failed to compile %%F
        pause
        exit /b 1
    )
)

echo.
echo All shaders successfully compiled!
pause
