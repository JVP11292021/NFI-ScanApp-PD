#include "PickingRenderSystem.hpp"
#include "descriptors.hpp"

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

void PickingRenderSystem::copyPixelToStaging(VkCommandBuffer cmdBuffer, uint32_t mouseX, uint32_t mouseY) {
    VkBufferImageCopy regionID{};
    regionID.bufferOffset = 0;
    regionID.bufferRowLength = 0;
    regionID.bufferImageHeight = 0;
    regionID.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    regionID.imageSubresource.mipLevel = 0;
    regionID.imageSubresource.baseArrayLayer = 0;
    regionID.imageSubresource.layerCount = 1;

    uint32_t flippedY = pickingFB.getExtent().height - mouseY - 1;

    regionID.imageOffset = {
        static_cast<int32_t>(mouseX),
        static_cast<int32_t>(flippedY),
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
        static_cast<int32_t>(flippedY),
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
    VLE_LOGI("Picked Object ID: ", std::to_string(pick.objectID).c_str(),
            " Point Index: ", std::to_string(pick.pointIndex).c_str()
    );

	result.id = (pick.objectID << 16) | (pick.pointIndex & 0xFFFF);

    stagingBufferPos->map();
    float pos[4];
    memcpy(pos, stagingBufferPos->getMappedMemory(), sizeof(float) * 4);
    stagingBufferPos->unmap();

    result.worldPos = glm::vec3(pos[0], pos[1], pos[2]);
    VLE_LOGI("Picked ID: ", std::to_string(result.id).c_str(),
            " at world position: ",
            std::to_string(result.worldPos.x).c_str(), ", ",
            std::to_string(result.worldPos.y).c_str(), ", ",
            std::to_string(result.worldPos.z).c_str()
    );
    return result;
}
