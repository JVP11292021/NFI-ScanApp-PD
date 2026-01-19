cmake_minimum_required(VERSION 3.16)
project(FreeImage VERSION 3.19.0 LANGUAGES C CXX)

# ------------------------------------------------------------------------------
# Options
# ------------------------------------------------------------------------------
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(BUILD_JXR "Build JPEG-XR support" WIN32)
option(BUILD_TESTS "Build the test suite" OFF)
option(BUILD_ZLIB "Build zlib from source or link" ON)
option(BUILD_LIBPNG "Build libpng from source or link" ON)
option(BUILD_LIBTIFF "Build libTIFF from source or link" ON)
option(BUILD_WEBP "Build webp from source or link" ON)
option(BUILD_LIBRAWLITE "Build librawlite from source or link" OFF)
option(BUILD_OPENEXR "Build OpenEXR from source or link" OFF)

if(NOT BUILD_SHARED_LIBS)
    add_compile_definitions(FREEIMAGE_LIB)
endif()

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

# ------------------------------------------------------------------------------
# Core FreeImage sources (ONLY .c / .cpp)
# ------------------------------------------------------------------------------
file(GLOB FREEIMAGE_CORE_SRC
        Source/FreeImage/*.cpp
        Source/FreeImage/*.c
        Source/Metadata/*.cpp
        Source/FreeImageToolkit/*.cpp
)

# ------------------------------------------------------------------------------
# JPEG
# ------------------------------------------------------------------------------
file(GLOB LIBJPEG_SRC
        Source/LibJPEG/*.c
)

# ------------------------------------------------------------------------------
# OpenJPEG
# ------------------------------------------------------------------------------
file(GLOB LIBOPENJPEG_SRC
        Source/LibOpenJPEG/*.c
)

set(FreeImage_SOURCES
        ${FREEIMAGE_CORE_SRC}
        ${LIBJPEG_SRC}
        ${LIBOPENJPEG_SRC}
)

# ------------------------------------------------------------------------------
# ZLIB
# ------------------------------------------------------------------------------
if(BUILD_ZLIB)
    file(GLOB ZLIB_SRC Source/ZLib/*.c)
    list(APPEND FreeImage_SOURCES ${ZLIB_SRC})
    add_compile_definitions(INCLUDE_LIB_ZLIB)
endif()

# ------------------------------------------------------------------------------
# PNG
# ------------------------------------------------------------------------------
if(BUILD_LIBPNG)
    file(GLOB LIBPNG_SRC Source/LibPNG/*.c)
    list(APPEND FreeImage_SOURCES ${LIBPNG_SRC})
endif()

# ------------------------------------------------------------------------------
# TIFF
# ------------------------------------------------------------------------------
if(BUILD_LIBTIFF)
    file(GLOB LIBTIFF_SRC Source/LibTIFF4/*.c)
    list(APPEND FreeImage_SOURCES ${LIBTIFF_SRC})
    add_compile_definitions(INCLUDE_LIB_TIFF4)
endif()

# ------------------------------------------------------------------------------
# WEBP
# ------------------------------------------------------------------------------
if(BUILD_WEBP)
    file(GLOB_RECURSE WEBP_SRC
            Source/LibWebP/src/*.c
    )
    list(APPEND FreeImage_SOURCES ${WEBP_SRC})
    add_compile_definitions(INCLUDE_LIB_WEBP)
endif()

# ------------------------------------------------------------------------------
# JPEG-XR (FIXED: NO DIRECTORY ENTRIES)
# ------------------------------------------------------------------------------
if(BUILD_JXR)
    file(GLOB_RECURSE JXR_SRC
            Source/LibJXR/image/**/*.c
            Source/LibJXR/jxrgluelib/*.c
    )
    list(APPEND FreeImage_SOURCES
            ${JXR_SRC}
            Source/FreeImage/PluginJXR.cpp
    )
    add_compile_definitions(INCLUDE_LIB_JXR)
endif()

# ------------------------------------------------------------------------------
# OpenEXR (HALF always required)
# ------------------------------------------------------------------------------
file(GLOB OPENEXR_HALF_SRC Source/OpenEXR/Half/*.cpp)
list(APPEND FreeImage_SOURCES ${OPENEXR_HALF_SRC})

if(BUILD_OPENEXR)
    file(GLOB_RECURSE OPENEXR_SRC
            Source/OpenEXR/Iex/*.cpp
            Source/OpenEXR/IexMath/*.cpp
            Source/OpenEXR/IlmImf/*.cpp
            Source/OpenEXR/IlmThread/*.cpp
            Source/OpenEXR/Imath/*.cpp
    )
    list(APPEND FreeImage_SOURCES ${OPENEXR_SRC})
    add_compile_definitions(INCLUDE_LIB_OPENEXR)
endif()

# ------------------------------------------------------------------------------
# LibRawLite
# ------------------------------------------------------------------------------
if(BUILD_LIBRAWLITE)
    file(GLOB_RECURSE LIBRAW_SRC Source/LibRawLite/src/*.cpp)
    list(APPEND FreeImage_SOURCES ${LIBRAW_SRC})
    add_compile_definitions(INCLUDE_LIB_RAW)
endif()

# ------------------------------------------------------------------------------
# Library
# ------------------------------------------------------------------------------
add_library(FreeImage ${FreeImage_SOURCES})

# ------------------------------------------------------------------------------
# Include directories
# ------------------------------------------------------------------------------
target_include_directories(FreeImage PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Source>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Source/Metadata>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Source/FreeImageToolkit>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Source/LibJPEG>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Source/LibOpenJPEG>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Source/ZLib>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Source/LibPNG>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Source/LibTIFF4>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Source/LibWebP>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Source/LibJXR>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Source/OpenEXR>

        $<INSTALL_INTERFACE:include>
)

# ------------------------------------------------------------------------------
# Compile definitions
# ------------------------------------------------------------------------------
target_compile_definitions(FreeImage PUBLIC
        DISABLE_PERF_MEASUREMENT
        OPJ_STATIC
        NO_LCMS
        NO_WINDOWS
)

# ------------------------------------------------------------------------------
# Platform specifics
# ------------------------------------------------------------------------------
if(MSVC)
    target_compile_options(FreeImage PRIVATE /MP)
endif()

if(WIN32)
    target_link_libraries(FreeImage ws2_32)
endif()

# ------------------------------------------------------------------------------
# Tests
# ------------------------------------------------------------------------------
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(TestAPI)
endif()

# ------------------------------------------------------------------------------
# Install
# ------------------------------------------------------------------------------
install(TARGETS FreeImage
        EXPORT FreeImageTargets
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
)

install(FILES Source/FreeImage.h DESTINATION include)

install(EXPORT FreeImageTargets
        FILE FreeImageTargets.cmake
        NAMESPACE FreeImage::
        DESTINATION lib/cmake/FreeImage
)
