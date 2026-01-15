//
// Created by jessy on 1/13/2026.
//

#ifndef IPMEDTH_NFI_ANDROIDENGINE_H
#define IPMEDTH_NFI_ANDROIDENGINE_H

#include "Semantics.h"

#include <android/asset_manager.h>

#include <EngineBackend/defs.hpp>
#include <EngineBackend/AndroidWindow.hpp>
#include <EngineBackend/Device.hpp>
#include <EngineBackend/Descriptors.hpp>
#include <EngineBackend/Model.hpp>
#include <EngineBackend/Object.hpp>

#include <Systems/Renderer.hpp>
#include <Systems/CameraSystem.hpp>
#include <Systems/ObjectRenderSystem.hpp>

using UboBuffer = vle::Buffer;

class AndroidEngine final {
public:
    explicit AndroidEngine(
            AAssetManager* assetManager,
            ANativeWindow* nativeWindow,
            std::int32_t width,
            std::int32_t height);
    ~AndroidEngine();

    NON_COPYABLE(AndroidEngine)
public:
    // TODO needs to be implemented
    void resize(std::int32_t width, std::int32_t height);
    void renderFrame(float dt);
    void waitForDevice();

    inline bool killLoop() { return this->_win.shouldClose(); }
    inline float getAspectRatio() { return this->_renderer.getAspectRatio(); }

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
    std::unique_ptr<vle::sys::ObjectRenderSystem> _objectRenderSystem;

private:
    std::unique_ptr<vle::DescriptorPool> globalPool{};
    vle::ObjectMap objects;
    vle::ObjectMap points;

};


#endif //IPMEDTH_NFI_ANDROIDENGINE_H
