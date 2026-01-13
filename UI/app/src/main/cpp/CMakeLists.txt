cmake_minimum_required(VERSION 3.22.1)
project(vulkanCore LANGUAGES C CXX)

# ------------------------ GLM -------------------------

add_library(glm INTERFACE)

target_include_directories(glm INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glm
)

# ------------------- tiny OBJ loader --------------------

add_library(tinyobjloader INTERFACE)

target_include_directories( tinyobjloader INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/tinyobjloader
)


# ------------------------ ASSIMP ------------------------
set(ASSIMP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/assimp)
set(ASSIMP_INCLUDE_DIR ${ASSIMP_ROOT})
set(ASSIMP_LIB_DIR ${ASSIMP_ROOT}/lib/${ANDROID_ABI})  # ABI-specific

# Include headers
#include_directories(${ASSIMP_INCLUDE_DIR})

add_library(assimp SHARED IMPORTED)
set_target_properties(assimp PROPERTIES
        IMPORTED_LOCATION ${ASSIMP_LIB_DIR}/libassimp.so
)

# If you used c++_shared, link libc++_shared too
find_library(cpp_shared c++_shared)

# --------------------- Engine Backend ---------------------

add_library(engineBackend STATIC
    thirdparty/VulkanLiteEngine/Engine/EngineBackend/AndroidWindow.cpp
    thirdparty/VulkanLiteEngine/Engine/EngineBackend/Buffer.cpp
    thirdparty/VulkanLiteEngine/Engine/EngineBackend/Camera.cpp
    thirdparty/VulkanLiteEngine/Engine/EngineBackend/Descriptors.cpp
    thirdparty/VulkanLiteEngine/Engine/EngineBackend/Device.cpp
#    thirdparty/VulkanLiteEngine/Engine/EngineBackend/HID.cpp
    thirdparty/VulkanLiteEngine/Engine/EngineBackend/Model.cpp
    thirdparty/VulkanLiteEngine/Engine/EngineBackend/Object.cpp
    thirdparty/VulkanLiteEngine/Engine/EngineBackend/Pipeline.cpp
    thirdparty/VulkanLiteEngine/Engine/EngineBackend/SwapChain.cpp
)

set_target_properties(engineBackend PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

target_link_libraries(engineBackend PUBLIC assimp glm tinyobjloader)

target_include_directories(engineBackend PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/VulkanLiteEngine/Engine/EngineUtils
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty
)

target_compile_definitions(engineBackend PRIVATE
        USE_VULKAN
)

# --------------------- Engine Utils ---------------------

add_library(engineUtils STATIC
        thirdparty/VulkanLiteEngine/Engine/EngineUtils/Aligned.cpp
        thirdparty/VulkanLiteEngine/Engine/EngineUtils/Hash.cpp
        thirdparty/VulkanLiteEngine/Engine/EngineUtils/VkHelpers.cpp
)

set_target_properties(engineUtils PROPERTIES
        POSITION_INDEPENDENT_CODE ON
)

target_link_libraries(engineUtils   PUBLIC glm)

target_include_directories(engineUtils PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/VulkanLiteEngine/Engine
)

target_compile_definitions(engineUtils PRIVATE
        USE_VULKAN
)

# ------------------- Engine Systems --------------------

add_library(engineSystem STATIC
        thirdparty/VulkanLiteEngine/Engine/Systems/CameraSystem.cpp
        thirdparty/VulkanLiteEngine/Engine/Systems/ObjectRenderSystem.cpp
        thirdparty/VulkanLiteEngine/Engine/Systems/PointCloudRenderSystem.cpp
        thirdparty/VulkanLiteEngine/Engine/Systems/PointLightSystem.cpp
        thirdparty/VulkanLiteEngine/Engine/Systems/Renderer.cpp
)

set_target_properties(engineSystem PROPERTIES
        POSITION_INDEPENDENT_CODE ON
)

target_link_libraries(engineSystem  PUBLIC glm)

target_include_directories(engineSystem PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/VulkanLiteEngine/Engine/EngineBackend
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/VulkanLiteEngine/Engine/Systems
)

# ----------------------- Picking ------------------------

#add_library(picking STATIC
#
#)

# -------------------- ENGINE BACKEND BRIDGE --------------------

add_library(engineBackendBridge SHARED
        bridge/EngineBackend/Buffer_bridge.cpp
        bridge/EngineBackend/Descriptors_bridge.cpp
        bridge/EngineBackend/NativeConstants_bridge.cpp
        bridge/EngineBackend/Window_bridge.cpp
)

target_include_directories(engineBackendBridge PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/VulkanLiteEngine/Engine
)

# -------------------- LINK EVERYTHING --------------------
find_library(vulkan-lib vulkan)

target_link_libraries(
        engineBackendBridge
        engineBackend
        engineUtils
        engineSystem
        android
        ${vulkan-lib}
        glm
        log
        ${cpp_shared}
)