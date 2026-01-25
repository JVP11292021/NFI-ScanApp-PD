//
// Created by jessy on 1/13/2026.
//

#ifndef IPMEDTH_NFI_ANDROID_ENGINE_H
#define IPMEDTH_NFI_ANDROID_ENGINE_H

#include "Semantics.h"
#include "Surface.h"

#include <android/asset_manager.h>

#include <EngineBackend/defs.hpp>
#include <EngineBackend/AndroidWindow.hpp>
#include <EngineBackend/Device.hpp>
#include <EngineBackend/Descriptors.hpp>
#include <EngineBackend/Model.hpp>
#include <EngineBackend/Object.hpp>

#include <Systems/Renderer.hpp>
#include <Systems/CameraSystem.hpp>
#include <Systems/RenderSystem.hpp>
#include <Systems/ObjectRenderSystem.hpp>
#include <Systems/PointCloudRenderSystem.hpp>
#include "RayTracing/MarkerManager.h"
#include "RayTracing/PickingFramebuffer.hpp"
#include "RayTracing/PickingRenderSystem.hpp"

using UboBuffer = vle::Buffer;

class AndroidEngine final : public IAndroidSurface {
public:
    explicit AndroidEngine(
            AAssetManager* assetManager,
            ANativeWindow* nativeWindow,
            std::int32_t width,
            std::int32_t height);
    ~AndroidEngine() override;

    NON_COPYABLE(AndroidEngine)
public:
    // TODO needs to be implemented
    void resize(std::int32_t width, std::int32_t height) override;
    void drawFrame() override;
    void onDrag(float dx, float dy);
    void onZoom(float scaleFactor);
    void onStrafe(float dx, float dy);
    void waitForDevice();
    void onTap(uint32_t x, uint32_t y);

//    inline bool killLoop() { return this->_win.shouldClose(); }
//    inline float getAspectRatio() { return this->_renderer.getAspectRatio(); }
private:
    void mapUniformBufferObjects();
    void makeDescriptorSets();
    void makeSystems();
    void loadObjects();

private:
    AAssetManager* _assetManager = nullptr;
    vle::AndroidWindow _win;
    vle::EngineDevice _device;
    vle::sys::Renderer _renderer;
    vle::sys::CameraSystem _cam;
    std::unique_ptr<vle::sys::PointCloudRenderSystem> pointCloudRenderSystem;
    std::unique_ptr<vle::sys::ObjectRenderSystem> objectRenderSystem;
    std::unique_ptr<PickingRenderSystem> pickingRenderSystem;

private:
    std::unique_ptr<vle::DescriptorPool> globalPool{};
    vle::ObjectMap objects;
    vle::ObjectMap points;
    std::chrono::steady_clock::time_point _currentTime;

private:
    MarkerManager markerManager;
    bool shouldPick = false;
    uint32_t pickX = 0, pickY = 0;
};


#endif //IPMEDTH_NFI_ANDROID_ENGINE_H
