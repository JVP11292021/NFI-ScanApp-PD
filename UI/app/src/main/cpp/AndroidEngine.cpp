//
// Created by jessy on 1/13/2026.
//

#include "AndroidEngine.h"

#include <EngineBackend/FrameInfo.hpp>
#include <cstdint>

static constexpr auto MAX_FRAMES_IN_FLIGHT = vle::EngineSwapChain::MAX_FRAMES_IN_FLIGHT;
static std::vector<std::unique_ptr<UboBuffer>> uboBuffers(MAX_FRAMES_IN_FLIGHT);
static std::unique_ptr<vle::DescriptorSetLayout> globalSetLayout;
static std::vector<VkDescriptorSet> globalDescriptorSets(MAX_FRAMES_IN_FLIGHT);

AndroidEngine::AndroidEngine(
        AAssetManager* assetManager,
        ANativeWindow* nativeWindow,
        std::int32_t width,
        std::int32_t height
)
  :
      _assetManager(assetManager),
      _win(nativeWindow, width, height, "NFI Scan App"),
      _device(_win, _assetManager),
      _renderer(_win, _device),
      _cam(
              glm::vec3(0.f, 0.f, 2.5f),
              glm::vec3(0.f, 0.f, 1.f)
      )
{
    this->globalPool = vle::DescriptorPool::Builder(this->_device)
            .setMaxSets(MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT)
            .build();

    this->loadObjects();
    this->mapUniformBufferObjects();
    this->makeDescriptorSets();
    this->makeSystems();
}

AndroidEngine::~AndroidEngine() {

}

void AndroidEngine::resize(ANativeWindow* nav_win) {
    if (!nav_win) {
        VLE_LOGE("The window could not be resized because No ANativeWindow was passed through");
        return;
    }

    this->_win.setNativeWindow(nav_win);
}

void AndroidEngine::mapUniformBufferObjects() {
    for (std::int32_t i = 0; i < uboBuffers.size(); i++) {
        uboBuffers[i] = std::make_unique<UboBuffer>(
                this->_device,
                sizeof(vle::GlobalUbo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        uboBuffers[i]->map();
    }
}

void AndroidEngine::makeDescriptorSets() {
    globalSetLayout = vle::DescriptorSetLayout::Builder(this->_device)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
            .build();

    for (std::int32_t i = 0; i < globalDescriptorSets.size(); i++) {
        auto bufferInfo = uboBuffers[i]->descriptorInfo();
        vle::DescriptorWriter(*globalSetLayout, *this->globalPool)
                .writeBuffer(0, &bufferInfo)
                .build(globalDescriptorSets[i]);
    }
}

void AndroidEngine::makeSystems() {
    this->_objectRenderSystem =
            std::make_unique<vle::sys::ObjectRenderSystem>(
                    this->_device,
                    this->_renderer.getSwapChainRenderPass(),
                    globalSetLayout->getDescriptorSetLayout(),
                    "shaders/simple_shader.vert.spv",
                    "shaders/simple_shader.frag.spv");
}

void AndroidEngine::loadObjects() {
    std::shared_ptr<vle::ShaderModel> model = vle::entity::Cube(this->_device, { .0f,.0f,.0f });
    auto cube = vle::Object::create();
    cube.model = model;
    cube.transform.translation = { .0f,.0f,.5f };
    cube.transform.scale = { .5f,.5f,.5f };
    this->objects.emplace(cube.getId(), std::move(cube));
}

void AndroidEngine::renderFrame(float frameTimeElapsed) {
    if (auto commandBuffer = _renderer.beginFrame()) {

        std::int32_t frameIndex = _renderer.getFrameIndex();

        vle::FrameInfo frameInfo{
                frameIndex,
                frameTimeElapsed,
                commandBuffer,
                globalDescriptorSets[frameIndex],
                this->objects,
                this->points
        };

        // === UPDATE ===
        vle::GlobalUbo ubo{};
        ubo.projection = this->_cam.getProjMatrix();
        ubo.view = this->_cam.getViewMatrix();
        ubo.inverseView = glm::inverse(ubo.view);
        ubo.ambientLightColor = glm::vec4(1.f, 1.f, 1.f, 1.f);
        ubo.numLights = 0;

        uboBuffers[frameIndex]->writeToBuffer(&ubo);
        uboBuffers[frameIndex]->flush();

        // === RENDER ===
        _renderer.beginSwapChainRenderPass(commandBuffer);
        this->_objectRenderSystem->render(frameInfo);
        // pointLightSystem.render(frameInfo);
        // pointCloudRenderSystem.render(frameInfo);

        _renderer.endSwapChainRenderPass(commandBuffer);
        _renderer.endFrame();
    }
}

void AndroidEngine::waitForDevice() {
    vkDeviceWaitIdle(this->_device.device());
}