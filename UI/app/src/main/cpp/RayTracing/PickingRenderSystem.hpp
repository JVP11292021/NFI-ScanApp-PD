#ifndef RT_PICKING_RENDER_SYSTEM_HPP
#define RT_PICKING_RENDER_SYSTEM_HPP

#include "engdefs.hpp"
#include "RenderSystem.hpp"
#include "PickingFramebuffer.hpp"

#include <iostream>

#include <Device.hpp>
#include <defs.hpp>
#include <Pipeline.hpp>
#include <Object.hpp>
#include <Camera.hpp>
#include <FrameInfo.hpp>
#include <Descriptors.hpp>

#include <array>

struct PickingPushConstantData {
    glm::mat4 modelMatrix{ 1.f };
    uint32_t objectID = 0;
    float pointSize = 1.0f;
    float padding = 0.0f;
};

struct PickResult {
    bool hit = false;
    uint32_t id = 0xFFFFFFFF;
    uint32_t objectID = 0xFFFFFFFF;
    uint32_t pointIndex = 0xFFFFFFFF;
    glm::vec3 worldPos = glm::vec3(0.0f);
};

struct PickID {
    uint32_t objectID;
    uint32_t pointIndex;
};

class PickingRenderSystem : public vle::sys::RenderSystem<PickingPushConstantData>
{
public:
    using Base = vle::sys::RenderSystem<PickingPushConstantData>;
    PickingRenderSystem(
            vle::EngineDevice& device,
            uint32_t width,
            uint32_t height,
            VkDescriptorSetLayout globalSetLayout,
            VkRenderPass renderPass,
            const std::string& vertPath,
            const std::string& fragPath
            );

    void update(vle::FrameInfo& frameInfo, vle::GlobalUbo& ubo) override;
    void render(vle::FrameInfo& frameInfo) override;

    void copyPixelToStaging(VkCommandBuffer cmdBuffer, uint32_t mouseX, uint32_t mouseY);
    PickResult readPickResult();

    PickingFramebuffer& getFramebuffer() { return pickingFB; }

private:
    void createPipeline(VkRenderPass renderPass, const std::string& vertPath, const std::string& fragPath);

private:
    PickingFramebuffer pickingFB;
    std::unique_ptr<vle::Buffer> stagingBufferID;
    std::unique_ptr<vle::Buffer> stagingBufferPos;

    // MRT color blend attachments - must persist for pipeline lifetime
    std::array<VkPipelineColorBlendAttachmentState, 2> colorBlendAttachments{};
};

#endif // !RT_PICKING_RENDER_SYSTEM_HPP
