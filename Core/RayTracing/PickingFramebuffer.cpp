#include "PickingFrameBuffer.hpp"
#include <stdexcept>
#include <array>

PickingFramebuffer::PickingFramebuffer(vle::EngineDevice& device, uint32_t width, uint32_t height)
    : device(device), extent{width, height} {
    createRenderPass();
    createImages();
    createFramebuffer();
}

PickingFramebuffer::~PickingFramebuffer() {
    vkDestroyFramebuffer(device.device(), framebuffer, nullptr);
    vkDestroyRenderPass(device.device(), renderPass, nullptr);

    vkDestroyImageView(device.device(), colorImageView, nullptr);
    vkDestroyImage(device.device(), colorImage, nullptr);
    vkFreeMemory(device.device(), colorImageMemory, nullptr);

    vkDestroyImageView(device.device(), positionImageView, nullptr);
    vkDestroyImage(device.device(), positionImage, nullptr);
    vkFreeMemory(device.device(), positionImageMemory, nullptr);

    vkDestroyImageView(device.device(), depthImageView, nullptr);
    vkDestroyImage(device.device(), depthImage, nullptr);
    vkFreeMemory(device.device(), depthImageMemory, nullptr);
}

void PickingFramebuffer::createRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = VK_FORMAT_R32G32_UINT;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

    VkAttachmentDescription positionAttachment{};
    positionAttachment.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    positionAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    positionAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    positionAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    positionAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    positionAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    positionAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    positionAttachment.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

    VkAttachmentDescription depthAttachment{};
    // Use the device's findSupportedFormat to get the depth format
    VkFormat depthFormat = device.findSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
    depthAttachment.format = depthFormat;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference positionAttachmentRef{};
    positionAttachmentRef.attachment = 1;
    positionAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 2;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    std::array<VkAttachmentReference, 2> colorAttachments = {colorAttachmentRef, positionAttachmentRef};

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = static_cast<uint32_t>(colorAttachments.size());
    subpass.pColorAttachments = colorAttachments.data();
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 3> attachments = {colorAttachment, positionAttachment, depthAttachment};

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(device.device(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create picking render pass!");
    }
}

void PickingFramebuffer::createImages() {
    // Create color image (ID) using device's createImageWithInfo
    VkImageCreateInfo colorImageInfo{};
    colorImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    colorImageInfo.imageType = VK_IMAGE_TYPE_2D;
    colorImageInfo.extent.width = extent.width;
    colorImageInfo.extent.height = extent.height;
    colorImageInfo.extent.depth = 1;
    colorImageInfo.mipLevels = 1;
    colorImageInfo.arrayLayers = 1;
    colorImageInfo.format = VK_FORMAT_R32G32_UINT;
    colorImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    colorImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    colorImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    colorImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    device.createImageWithInfo(colorImageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colorImage, colorImageMemory);

    VkImageViewCreateInfo colorViewInfo{};
    colorViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    colorViewInfo.image = colorImage;
    colorViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    colorViewInfo.format = VK_FORMAT_R32G32_UINT;
    colorViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    colorViewInfo.subresourceRange.baseMipLevel = 0;
    colorViewInfo.subresourceRange.levelCount = 1;
    colorViewInfo.subresourceRange.baseArrayLayer = 0;
    colorViewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device.device(), &colorViewInfo, nullptr, &colorImageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create color image view!");
    }

    // Create position image
    VkImageCreateInfo positionImageInfo{};
    positionImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    positionImageInfo.imageType = VK_IMAGE_TYPE_2D;
    positionImageInfo.extent.width = extent.width;
    positionImageInfo.extent.height = extent.height;
    positionImageInfo.extent.depth = 1;
    positionImageInfo.mipLevels = 1;
    positionImageInfo.arrayLayers = 1;
    positionImageInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    positionImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    positionImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    positionImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    positionImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    positionImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    device.createImageWithInfo(positionImageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, positionImage, positionImageMemory);

    VkImageViewCreateInfo positionViewInfo{};
    positionViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    positionViewInfo.image = positionImage;
    positionViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    positionViewInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    positionViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    positionViewInfo.subresourceRange.baseMipLevel = 0;
    positionViewInfo.subresourceRange.levelCount = 1;
    positionViewInfo.subresourceRange.baseArrayLayer = 0;
    positionViewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device.device(), &positionViewInfo, nullptr, &positionImageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create position image view!");
    }

    // Create depth image
    VkFormat depthFormat = device.findSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );

    VkImageCreateInfo depthImageInfo{};
    depthImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    depthImageInfo.imageType = VK_IMAGE_TYPE_2D;
    depthImageInfo.extent.width = extent.width;
    depthImageInfo.extent.height = extent.height;
    depthImageInfo.extent.depth = 1;
    depthImageInfo.mipLevels = 1;
    depthImageInfo.arrayLayers = 1;
    depthImageInfo.format = depthFormat;
    depthImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    depthImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    depthImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    depthImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    device.createImageWithInfo(depthImageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);

    VkImageViewCreateInfo depthViewInfo{};
    depthViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    depthViewInfo.image = depthImage;
    depthViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    depthViewInfo.format = depthFormat;
    depthViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    depthViewInfo.subresourceRange.baseMipLevel = 0;
    depthViewInfo.subresourceRange.levelCount = 1;
    depthViewInfo.subresourceRange.baseArrayLayer = 0;
    depthViewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device.device(), &depthViewInfo, nullptr, &depthImageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create depth image view!");
    }
}

void PickingFramebuffer::createFramebuffer() {
    std::array<VkImageView, 3> attachments = {
        colorImageView,
        positionImageView,
        depthImageView
    };

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = extent.width;
    framebufferInfo.height = extent.height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(device.device(), &framebufferInfo, nullptr, &framebuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create picking framebuffer!");
    }
}
