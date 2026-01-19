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

using UboBuffer = vle::Buffer;

struct CubePushConstants {
    glm::mat4 pvm{ 1.f };
};

class CubeRenderSystem : public vle::sys::RenderSystem<CubePushConstants> {
public:
    CubeRenderSystem(
        vle::EngineDevice& device,
        VkRenderPass renderPass,
        VkDescriptorSetLayout globalSetLayout,
        const std::string& vertPath,
        const std::string& fragPath);

    NON_COPYABLE(CubeRenderSystem)

public:
    void update(vle::FrameInfo& frameInfo, vle::GlobalUbo& ubo) override;
    void render(vle::FrameInfo& frameInfo) override;

private:
    void createPipeline(
            VkRenderPass renderPass,
            const std::string& vertPath,
            const std::string& fragPath) override;
};

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

    void waitForDevice();

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
    std::unique_ptr<CubeRenderSystem> _cubeRenderSystem;

private:
    std::unique_ptr<vle::DescriptorPool> globalPool{};
    vle::ObjectMap objects;
    vle::ObjectMap points;
    std::chrono::steady_clock::time_point _currentTime;
};


#endif //IPMEDTH_NFI_ANDROID_ENGINE_H
