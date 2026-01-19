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

CubeRenderSystem::CubeRenderSystem(
        vle::EngineDevice& device,
        VkRenderPass renderPass,
        VkDescriptorSetLayout globalSetLayout,
        const std::string& vertPath,
        const std::string& fragPath
)
        : RenderSystem(device, globalSetLayout)
{
    this->createPipeline(renderPass, vertPath, fragPath);
}

void CubeRenderSystem::update(vle::FrameInfo& frameInfo, vle::GlobalUbo& ubo) {}

void CubeRenderSystem::render(vle::FrameInfo& frameInfo) {
    this->pipeline->bind(frameInfo.commandBuffer);

    vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            this->pipelineLayout,
            0, 1,
            &frameInfo.globalDescriptorSet,
            0, nullptr );

    for (auto& kv: frameInfo.gameObjects) {
        vle::Object& obj = kv.second;
        if (!obj.model) continue;
        CubePushConstants push{};
        push.pvm = frameInfo.pvmMatrix;
        vkCmdPushConstants(frameInfo.commandBuffer, this->pipelineLayout, VLE_PUSH_CONST_VERT_FRAG_FLAG, 0, sizeof(CubePushConstants), &push);
        obj.model->bind(frameInfo.commandBuffer);
        obj.model->draw(frameInfo.commandBuffer);
    }

}

void CubeRenderSystem::createPipeline(
        VkRenderPass renderPass,
        const std::string& vertPath,
        const std::string& fragPath
) {
    assert(this->pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

    vle::PipelineConfigInfo pipelineConfig{};
    vle::Pipeline::defaultPipelineConfigInfo(
            pipelineConfig);
    pipelineConfig.bindingDescriptions = vle::ShaderModel::Vertex::getBindingDescription();
    pipelineConfig.attributeDescriptors = vle::ShaderModel::Vertex::getAttributeDescription();
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = this->pipelineLayout;
    this->pipeline = std::make_unique<vle::Pipeline>(
            this->device, vertPath, fragPath, pipelineConfig);
}

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
    this->_cubeRenderSystem =
            std::make_unique<CubeRenderSystem>(
                    this->_device,
                    this->_renderer.getSwapChainRenderPass(),
                    globalSetLayout->getDescriptorSetLayout(),
                    "shaders/15-draw_cube.vert.spv",
                    "shaders/15-draw_cube.frag.spv");
}

void AndroidEngine::loadObjects() {
    std::shared_ptr<vle::ShaderModel> model = vle::entity::Cube(this->_device);
    auto cube = vle::Object::create();
    cube.model = model;
//    cube.transform.translation = { .0f,.0f,.5f };
//    cube.transform.scale = { .5f,.5f,.5f };
    this->objects.emplace(cube.getId(), std::move(cube));
}

void AndroidEngine::drawFrame() {
    vle::sys::CameraSystem cam{
            glm::vec3(0.f, 0.f, 2.5f),
            glm::vec3(0.f, 0.f, 1.f)
    };

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

        auto projectionViewModel = projection * view * model;

        vle::FrameInfo frameInfo{
                frameIndex,
                frameTimeElapsed,
                commandBuffer,
                globalDescriptorSets[frameIndex],
                this->objects,
                this->points,
                projectionViewModel
        };

        // === UPDATE ===
        vle::GlobalUbo ubo{};
        ubo.view = view;
        ubo.projection = projection;
        ubo.inverseView = inverseView;

        ubo.ambientLightColor = glm::vec4(1.f, 1.f, 1.f, 1.f);
        ubo.numLights = 0;

        uboBuffers[frameIndex]->writeToBuffer(&ubo);
        uboBuffers[frameIndex]->flush();

        // === RENDER ===
        _renderer.beginSwapChainRenderPass(commandBuffer);
        this->_cubeRenderSystem->render(frameInfo);
        // pointLightSystem.render(frameInfo);
        // pointCloudRenderSystem.render(frameInfo);

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
    constexpr float STRAFE_SENSITIVITY = 0.0006f;
    _cam.processKeyboard(vle::sys::RIGHT, dx * STRAFE_SENSITIVITY);
    _cam.processKeyboard(vle::sys::MOVE_UP, dy * STRAFE_SENSITIVITY);
}

void AndroidEngine::onZoom(float scaleFactor) {
    constexpr float ZOOM_SENSITIVITY = 0.2f;
    float zoomDelta = std::log(scaleFactor) * ZOOM_SENSITIVITY;
    _cam.processKeyboard(vle::sys::FORWARD, zoomDelta);
}