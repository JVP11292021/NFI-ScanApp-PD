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
    IAndroidSurface(),
    _assetManager(assetManager),
    _win(nativeWindow, width, height, "NFI Scan App"),
    _device(_win, _assetManager),
    _renderer(_win, _device)
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

AndroidEngine::~AndroidEngine() = default;

void AndroidEngine::resize(std::int32_t width, std::int32_t height) {
    this->_win.setSize(width, height);

}

void AndroidEngine::mapUniformBufferObjects() {
    for (auto & uboBuffer : uboBuffers) {
        uboBuffer = std::make_unique<UboBuffer>(
                this->_device,
                sizeof(vle::GlobalUbo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        uboBuffer->map();
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
    pointCloudRenderSystem =
            std::make_unique<vle::sys::PointCloudRenderSystem>(
                _device,
                _renderer.getSwapChainRenderPass(),
                globalSetLayout->getDescriptorSetLayout(),
                "shaders/point_cloud_shader.vert.spv",
                "shaders/point_cloud_shader.frag.spv"
            );

    pickingRenderSystem =
            std::make_unique<PickingRenderSystem>(
                _device,
                _win.getWidth(),
                _win.getHeight(),
                globalSetLayout->getDescriptorSetLayout(),
                _renderer.getSwapChainRenderPass(),
                "shaders/index_shader.vert.spv",
                "shaders/index_shader.frag.spv"
            );

    objectRenderSystem =
            std::make_unique<vle::sys::ObjectRenderSystem>(
                _device,
                _renderer.getSwapChainRenderPass(),
                globalSetLayout->getDescriptorSetLayout(),
                "shaders/simple_shader.vert.spv",
                "shaders/simple_shader.frag.spv"
            );
}

void AndroidEngine::loadObjects() {
    this->markerManager.loadMarkersFromTxt("cpp/markers.txt", _device, this->objects);

    std::shared_ptr<vle::ShaderModel> roomModel =
            vle::ShaderModel::createModelFromFile(_device, "simple_scene.ply");
    auto room = vle::Object::create();
    room.model = roomModel;
    room.transform.translation = { 0.f, .5f, 8.f };
    room.transform.rotation = {
            glm::radians(9.0f),
            glm::radians(180.f),
            glm::radians(93.0f)
    };
    this->points.emplace(room.getId(), std::move(room));
}

void AndroidEngine::drawFrame() {
    auto view = _cam.getViewMatrix();
    auto projection = _cam.getProjMatrix();
    auto inverseView = glm::inverse(view);

    auto newTime = std::chrono::high_resolution_clock::now();
    float frameTimeElapsed =
            std::chrono::duration<float, std::chrono::seconds::period>(newTime - this->_currentTime).count();
    this->_currentTime = newTime;

    constexpr auto MAX_FRAME_TIME_ELAPSED = 10000.f;
    frameTimeElapsed = glm::min(frameTimeElapsed, MAX_FRAME_TIME_ELAPSED);

    auto aspect = this->_renderer.getAspectRatio();

    if (auto commandBuffer = _renderer.beginFrame()) {

        std::int32_t frameIndex = _renderer.getFrameIndex();

        float fov = glm::radians(45.0f);
        if (this->_win.getWidth() > this->_win.getHeight()) {
            fov *= static_cast<float>(this->_win.getHeight()) / static_cast<float>(this->_win.getWidth());
        }

        auto model = glm::mat4(1.0f);
        // Vulkan clip space has inverted Y and half Z.
//        auto clip = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.5f, 1.0f);


        vle::FrameInfo frameInfo{
                frameIndex,
                frameTimeElapsed,
                commandBuffer,
                globalDescriptorSets[frameIndex],
                this->objects,
                this->points,
        };

        // === UPDATE ===
        vle::GlobalUbo ubo{};
        ubo.view = view;
        ubo.projection = projection;
        ubo.inverseView = inverseView;

        ubo.ambientLightColor = glm::vec4(1.f, 1.f, 1.f, 1.f);
        ubo.numLights = 0;

        pickingRenderSystem->update(frameInfo, ubo);
        this->markerManager.updateMarkerRotations(_cam.getPosition(), objects);

        uboBuffers[frameIndex]->writeToBuffer(&ubo);
        uboBuffers[frameIndex]->flush();

        pickingRenderSystem->render(frameInfo);
        if (shouldPick) {
            VkCommandBuffer pickCmdBuffer = _device.beginSingleTimeCommands();
            pickingRenderSystem->copyPixelToStaging(pickCmdBuffer, pickX, pickY);
            _device.endSingleTimeCommands(pickCmdBuffer);

            PickResult pick = pickingRenderSystem->readPickResult();

            if (pick.id != 0xFFFFFFFF) {
                this->markerManager.createMarker(pick.worldPos, _device, this->objects);
            }
            shouldPick = false;
        }

        // === RENDER ===
        _renderer.beginSwapChainRenderPass(commandBuffer);

        objectRenderSystem->render(frameInfo);
        pointCloudRenderSystem->render(frameInfo);
        // pointLightSystem.render(frameInfo);

        _renderer.endSwapChainRenderPass(commandBuffer);
        _renderer.endFrame();
    }
}

void AndroidEngine::waitForDevice() {
    vkDeviceWaitIdle(this->_device.device());
}

void AndroidEngine::onDrag(float dx, float dy) {
    constexpr float TOUCH_SENSITIVITY = 0.5f;
    _cam.processMouseInput(
            dy * TOUCH_SENSITIVITY,
            -dx * TOUCH_SENSITIVITY,
            true
    );
}

void AndroidEngine::onStrafe(float dx, float dy) {
    constexpr float STRAFE_SENSITIVITY = 0.002f;
    _cam.processKeyboard(vle::sys::RIGHT, dx * STRAFE_SENSITIVITY);
    _cam.processKeyboard(vle::sys::MOVE_UP, dy * STRAFE_SENSITIVITY);
}

void AndroidEngine::onZoom(float scaleFactor) {
    constexpr float ZOOM_SENSITIVITY = 0.5f;
    float zoomDelta = std::log(scaleFactor) * ZOOM_SENSITIVITY;
    _cam.processKeyboard(vle::sys::FORWARD, zoomDelta);
}

void AndroidEngine::onTap(uint32_t x, uint32_t y) {
    this->pickX = x;
    this->pickY = y;
    this->shouldPick = true;
}
