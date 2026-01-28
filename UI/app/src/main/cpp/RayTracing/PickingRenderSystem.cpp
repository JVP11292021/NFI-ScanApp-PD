#include "PickingRenderSystem.hpp"
#include <array>


//TODO: Add area selection picking support
// Achieve this by allowing the user to pick 4 points to form a rectangle


PickingRenderSystem::PickingRenderSystem(
        vle::EngineDevice& device,
        uint32_t width,
        uint32_t height,
        VkDescriptorSetLayout globalSetLayout,
        VkRenderPass renderPass,
        const std::string& vertPath,
        const std::string& fragPath
        )
    : Base(device, globalSetLayout),
    pickingFB(device, width, height)
{
    stagingBufferID = std::make_unique<vle::Buffer>(
        device,
        sizeof(PickID),
        1,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );
    
    stagingBufferPos = std::make_unique<vle::Buffer>(
        device,
        sizeof(float) * 4,
        1,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );
    
    this->createPipeline(renderPass, vertPath, fragPath);
    this->createTrianglePipeline(renderPass, vertPath, fragPath);
}

void PickingRenderSystem::update(vle::FrameInfo& frameInfo, vle::GlobalUbo& ubo) {
}

void PickingRenderSystem::render(vle::FrameInfo& frameInfo) {
    VkCommandBuffer cmdBuffer = frameInfo.commandBuffer;

    std::array<VkClearValue, 3> clearValues{};
    clearValues[0].color.uint32[0] = 0xFFFFFFFF;
    clearValues[0].color.uint32[1] = 0xFFFFFFFF;
    clearValues[1].color = { {0.f, 0.f, 0.f, 0.f} };
    clearValues[2].depthStencil = { 1.0f, 0 };


    VkRenderPassBeginInfo rpInfo{};
    rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpInfo.renderPass = pickingFB.getRenderPass();
    rpInfo.framebuffer = pickingFB.getFramebuffer();
    rpInfo.renderArea = { {0,0}, pickingFB.getExtent() };
    rpInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    rpInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(cmdBuffer, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

    pipeline->bind(cmdBuffer);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(pickingFB.getExtent().width);
    viewport.height = static_cast<float>(pickingFB.getExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

    VkRect2D scissor{ {0, 0}, pickingFB.getExtent() };
    vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

    vkCmdBindDescriptorSets(
        cmdBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout,
        0, 1,
        &frameInfo.globalDescriptorSet,
        0, nullptr);

    // Render point cloud objects with point pipeline
    for (auto& kv : frameInfo.pointCloud) {
        vle::Object& obj = kv.second;
        if (!obj.model) continue;

        PickingPushConstantData push{};
        push.modelMatrix = obj.transform.mat4();
        push.objectID = obj.getId();
        push.pointSize = 10.0f;

        vkCmdPushConstants(
            cmdBuffer,
            pipelineLayout,
            VLE_PUSH_CONST_VERT_FRAG_FLAG,
            0,
            sizeof(PickingPushConstantData),
            &push
        );

        obj.model->bind(cmdBuffer);
        obj.model->draw(cmdBuffer);
    }

    if (trianglePipeline) {
        trianglePipeline->bind(cmdBuffer);

        vkCmdBindDescriptorSets(
            cmdBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout,
            0, 1,
            &frameInfo.globalDescriptorSet,
            0, nullptr);

        for (auto& kv : frameInfo.gameObjects) {
            vle::Object& obj = kv.second;
            if (!obj.model) continue;

            PickingPushConstantData push{};
            push.modelMatrix = obj.transform.mat4();
            push.objectID = obj.getId();
            push.pointSize = 1.0f;

            vkCmdPushConstants(
                cmdBuffer,
                pipelineLayout,
                VLE_PUSH_CONST_VERT_FRAG_FLAG,
                0,
                sizeof(PickingPushConstantData),
                &push
            );

            obj.model->bind(cmdBuffer);
            obj.model->draw(cmdBuffer);
        }
    }

    vkCmdEndRenderPass(cmdBuffer);
}

void PickingRenderSystem::createPipeline(VkRenderPass renderPass, const std::string& vertPath, const std::string& fragPath) {
    assert(this->pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

    vle::PipelineConfigInfo pipelineConfig{};
    vle::Pipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.assemblyInputInfo.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    pipelineConfig.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
    
    auto bindings = vle::ShaderModel::Vertex::getBindingDescription();
    auto allAttributes = vle::ShaderModel::Vertex::getAttributeDescription();
    
    std::vector<VkVertexInputAttributeDescription> filteredAttributes;
    for (const auto& attr : allAttributes) {
        if (attr.location == 0) {
            filteredAttributes.push_back(attr);
        }
    }

    for (auto& attachment : colorBlendAttachments) {
        attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                    VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        attachment.blendEnable = VK_FALSE;
    }

    // Override the color blend info to use 2 attachments
    pipelineConfig.colorBlendInfo.attachmentCount = static_cast<uint32_t>(colorBlendAttachments.size());
    pipelineConfig.colorBlendInfo.pAttachments = colorBlendAttachments.data();

    //pipelineConfig.depthStencilInfo.depthTestEnable = VK_FALSE;
    //pipelineConfig.depthStencilInfo.depthWriteEnable = VK_FALSE;
    pipelineConfig.bindingDescriptions = bindings;
    pipelineConfig.attributeDescriptors = filteredAttributes;
    pipelineConfig.renderPass = pickingFB.getRenderPass();
    pipelineConfig.pipelineLayout = this->pipelineLayout;

    this->pipeline = std::make_unique<vle::Pipeline>(
        device,
        vertPath,
        fragPath,
        pipelineConfig
    );
}

void PickingRenderSystem::createTrianglePipeline(VkRenderPass renderPass, const std::string& vertPath, const std::string& fragPath) {
    assert(this->pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

    vle::PipelineConfigInfo pipelineConfig{};
    vle::Pipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.assemblyInputInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipelineConfig.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;

    auto bindings = vle::ShaderModel::Vertex::getBindingDescription();
    auto allAttributes = vle::ShaderModel::Vertex::getAttributeDescription();

    std::vector<VkVertexInputAttributeDescription> filteredAttributes;
    for (const auto& attr : allAttributes) {
        if (attr.location == 0) {  // position
            filteredAttributes.push_back(attr);
        }
    }

    // Configure 2 color blend attachments for MRT
    for (auto& attachment : colorBlendAttachments) {
        attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                    VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        attachment.blendEnable = VK_FALSE;
    }

    pipelineConfig.colorBlendInfo.attachmentCount = static_cast<uint32_t>(colorBlendAttachments.size());
    pipelineConfig.colorBlendInfo.pAttachments = colorBlendAttachments.data();

    pipelineConfig.bindingDescriptions = bindings;
    pipelineConfig.attributeDescriptors = filteredAttributes;
    pipelineConfig.renderPass = pickingFB.getRenderPass();
    pipelineConfig.pipelineLayout = this->pipelineLayout;

    this->trianglePipeline = std::make_unique<vle::Pipeline>(
        device,
        vertPath,
        fragPath,
        pipelineConfig
    );
}

void PickingRenderSystem::copyPixelToStaging(VkCommandBuffer cmdBuffer, uint32_t mouseX, uint32_t mouseY) {
    uint32_t fbWidth = pickingFB.getExtent().width;
    uint32_t fbHeight = pickingFB.getExtent().height;

    if (mouseX >= fbWidth) mouseX = fbWidth - 1;
    if (mouseY >= fbHeight) mouseY = fbHeight - 1;

    // Memory barrier to ensure render pass writes are complete and images are in correct layout
    VkImageMemoryBarrier barrierColorPre{};
    barrierColorPre.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrierColorPre.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrierColorPre.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrierColorPre.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrierColorPre.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrierColorPre.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrierColorPre.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrierColorPre.image = pickingFB.getColorImage();
    barrierColorPre.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrierColorPre.subresourceRange.baseMipLevel = 0;
    barrierColorPre.subresourceRange.levelCount = 1;
    barrierColorPre.subresourceRange.baseArrayLayer = 0;
    barrierColorPre.subresourceRange.layerCount = 1;

    VkImageMemoryBarrier barrierPosPre{};
    barrierPosPre.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrierPosPre.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrierPosPre.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrierPosPre.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrierPosPre.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrierPosPre.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrierPosPre.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrierPosPre.image = pickingFB.getPositionImage();
    barrierPosPre.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrierPosPre.subresourceRange.baseMipLevel = 0;
    barrierPosPre.subresourceRange.levelCount = 1;
    barrierPosPre.subresourceRange.baseArrayLayer = 0;
    barrierPosPre.subresourceRange.layerCount = 1;

    std::array<VkImageMemoryBarrier, 2> preBarriers = {barrierColorPre, barrierPosPre};

    vkCmdPipelineBarrier(
        cmdBuffer,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        static_cast<uint32_t>(preBarriers.size()), preBarriers.data()
    );

    VkBufferImageCopy regionID{};
    regionID.bufferOffset = 0;
    regionID.bufferRowLength = 0;
    regionID.bufferImageHeight = 0;
    regionID.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    regionID.imageSubresource.mipLevel = 0;
    regionID.imageSubresource.baseArrayLayer = 0;
    regionID.imageSubresource.layerCount = 1;

    // On Android, both touch coordinates and Vulkan image coordinates have Y=0 at the top
    // No Y-flip needed (unlike desktop GLFW where Y=0 is at the bottom)
    // TODO: Add Desktop vs Android check to invert Y only on Desktop
    uint32_t finalY = mouseY;

    regionID.imageOffset = {
        static_cast<int32_t>(mouseX),
        static_cast<int32_t>(finalY),
        0
    };
    regionID.imageExtent = { 1, 1, 1 };

    vkCmdCopyImageToBuffer(
        cmdBuffer,
        pickingFB.getColorImage(),
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        stagingBufferID->getBuffer(),
        1,
        &regionID
    );

    VkBufferImageCopy regionPos{};
    regionPos.bufferOffset = 0;
    regionPos.bufferRowLength = 0;
    regionPos.bufferImageHeight = 0;
    regionPos.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    regionPos.imageSubresource.mipLevel = 0;
    regionPos.imageSubresource.baseArrayLayer = 0;
    regionPos.imageSubresource.layerCount = 1;
    regionPos.imageOffset = {
        static_cast<int32_t>(mouseX),
        static_cast<int32_t>(finalY),
        0
    };
    regionPos.imageExtent = { 1, 1, 1 };

    vkCmdCopyImageToBuffer(
        cmdBuffer,
        pickingFB.getPositionImage(),
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        stagingBufferPos->getBuffer(),
        1,
        &regionPos
    );

    // Memory barrier to ensure GPU writes to staging buffers are visible to CPU
    VkMemoryBarrier memoryBarrier{};
    memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    memoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    memoryBarrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;

    vkCmdPipelineBarrier(
        cmdBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_HOST_BIT,
        0,
        1, &memoryBarrier,
        0, nullptr,
        0, nullptr
    );
}

PickResult PickingRenderSystem::readPickResult() {
    PickResult result{};

    PickID pick{};

    stagingBufferID->map();
    memcpy(&pick, stagingBufferID->getMappedMemory(), sizeof(PickID));
    stagingBufferID->unmap();

    if (pick.objectID == 0xFFFFFFFF) {
        result.hit = false;
		result.id = 0xFFFFFFFF;
        return result;
    }

    result.hit = true;
    result.objectID = pick.objectID;
    result.pointIndex = pick.pointIndex;

	result.id = (pick.objectID << 16) | (pick.pointIndex & 0xFFFF);

    stagingBufferPos->map();
    float pos[4];
    memcpy(pos, stagingBufferPos->getMappedMemory(), sizeof(float) * 4);
    stagingBufferPos->unmap();

    result.worldPos = glm::vec3(pos[0], pos[1], pos[2]);
    return result;
}
