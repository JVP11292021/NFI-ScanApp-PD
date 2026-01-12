#ifndef RT_PICKING_FRAMEBUFFER_HPP
#define RT_PICKING_FRAMEBUFFER_HPP

#include <defs.hpp>
#include <Device.hpp>
#include <memory>
#include <array>

class PickingFramebuffer {
public:
    PickingFramebuffer(vle::EngineDevice& device, uint32_t width, uint32_t height);
    ~PickingFramebuffer();

    PickingFramebuffer(const PickingFramebuffer&) = delete;
    PickingFramebuffer& operator=(const PickingFramebuffer&) = delete;

    VkRenderPass getRenderPass() const { return renderPass; }
    VkFramebuffer getFramebuffer() const { return framebuffer; }
    VkExtent2D getExtent() const { return extent; }
    
    VkImage getColorImage() const { return colorImage; }
    VkImage getPositionImage() const { return positionImage; }
    VkImage getDepthImage() const { return depthImage; }

private:
    void createRenderPass();
    void createImages();
    void createFramebuffer();

private:
    vle::EngineDevice& device;
    VkExtent2D extent;

    VkImage colorImage = VK_NULL_HANDLE;
    VkDeviceMemory colorImageMemory = VK_NULL_HANDLE;
    VkImageView colorImageView = VK_NULL_HANDLE;

    VkImage positionImage = VK_NULL_HANDLE;
    VkDeviceMemory positionImageMemory = VK_NULL_HANDLE;
    VkImageView positionImageView = VK_NULL_HANDLE;

    VkImage depthImage = VK_NULL_HANDLE;
    VkDeviceMemory depthImageMemory = VK_NULL_HANDLE;
    VkImageView depthImageView = VK_NULL_HANDLE;

    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
};

#endif // !RT_PICKING_FRAMEBUFFER_HPP
