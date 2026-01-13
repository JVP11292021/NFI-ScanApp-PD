cmake_minimum_required(VERSION 3.22)
project(VLECore LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------- Common flags ----------
add_compile_definitions(
    VLE_PLATFORM_ANDROID
    VK_USE_PLATFORM_ANDROID_KHR
)

# ---------- EngineBackend ----------
add_library(EngineBackend STATIC
    Engine/Engine/EngineBackend/AndroidWindow.cpp
    Engine/Engine/EngineBackend/AndroidWindow.hpp
    Engine/Engine/EngineBackend/Buffer.cpp
    Engine/Engine/EngineBackend/Buffer.hpp
    Engine/Engine/EngineBackend/Camera.cpp
    Engine/Engine/EngineBackend/Camera.hpp
    Engine/Engine/EngineBackend/defs.hpp
    Engine/Engine/EngineBackend/Descriptors.cpp
    Engine/Engine/EngineBackend/Descriptors.hpp
    Engine/Engine/EngineBackend/Device.cpp
    Engine/Engine/EngineBackend/Device.hpp
    Engine/Engine/EngineBackend/FrameInfo.hpp
    Engine/Engine/EngineBackend/GLFWWindow.cpp
    Engine/Engine/EngineBackend/GLFWWindow.hpp
    Engine/Engine/EngineBackend/HID.cpp
    Engine/Engine/EngineBackend/HID.hpp
    Engine/Engine/EngineBackend/Model.cpp
    Engine/Engine/EngineBackend/Model.hpp
    Engine/Engine/EngineBackend/Object.cpp
    Engine/Engine/EngineBackend/Object.hpp
    Engine/Engine/EngineBackend/Pipeline.cpp
    Engine/Engine/EngineBackend/Pipeline.hpp
    Engine/Engine/EngineBackend/SwapChain.cpp
    Engine/Engine/EngineBackend/SwapChain.hpp
    Engine/Engine/EngineBackend/Window.cpp
    Engine/Engine/EngineBackend/Window.hpp
)

target_include_directories(EngineBackend PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Engine/Engine
    ${CMAKE_CURRENT_SOURCE_DIR}/Libraries/include
)

# ---------- EngineSystems ----------
add_library(EngineSystems STATIC
    Engine/Engine/Systems/CameraSystem.cpp
    Engine/Engine/Systems/CameraSystem.hpp
    Engine/Engine/Systems/engdefs.hpp
    Engine/Engine/Systems/ObjectRenderSystem.cpp
    Engine/Engine/Systems/ObjectRenderSystem.hpp
    Engine/Engine/Systems/PointCloudRenderSystem.cpp
    Engine/Engine/Systems/PointCloudRenderSystem.hpp
    Engine/Engine/Systems/PointLightSystem.cpp
    Engine/Engine/Systems/PointLightSystem.hpp
    Engine/Engine/Systems/Renderer.cpp
    Engine/Engine/Systems/Renderer.hpp
    Engine/Engine/Systems/RenderSystem.hpp
)

target_link_libraries(EngineSystems
    PUBLIC EngineBackend
)

# ---------- EngineUtils ----------
add_library(EngineUtils STATIC
    Engine/Engine/EngineUtils/Aligned.cpp
    Engine/Engine/EngineUtils/Aligned.hpp
    Engine/Engine/EngineUtils/eutils.hpp
    Engine/Engine/EngineUtils/Hash.cpp
    Engine/Engine/EngineUtils/Hash.hpp
    Engine/Engine/EngineUtils/UtilsDefs.hpp
    Engine/Engine/EngineUtils/VkHelpers.cpp
    Engine/Engine/EngineUtils/VkHelpers.hpp
)

target_link_libraries(EngineUtils
    PUBLIC EngineBackend
)

add_library(Raytracing STATIC
    RayTracing/MarkerManager.cpp
    RayTracing/MarkerManager.h
    RayTracing/PickingFramebuffer.cpp
    RayTracing/PickingFramebuffer.hpp
    RayTracing/PickingRenderSystem.cpp
    RayTracing/PickingRenderSystem.hpp
)

# ---------- Aggregate target (optional but recommended) ----------
add_library(VLECore STATIC)

target_link_libraries(VLECore
    PUBLIC
        EngineBackend
        EngineSystems
        EngineUtils
)
