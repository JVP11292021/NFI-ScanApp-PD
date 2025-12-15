#ifndef GSPLATS_RENDER_SYSTEM_H
#define GSPLATS_RENDER_SYSTEM_H

#include <RenderSystem.hpp>

class SplatRenderSystem : public vle::sys::RenderSystem<> {
public:
	using Base = vle::sys::RenderSystem<>;
	SplatRenderSystem(vle::EngineDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);

	SplatRenderSystem(const SplatRenderSystem&) = delete;
	SplatRenderSystem& operator=(const SplatRenderSystem&) = delete;

public:
	void update(vle::FrameInfo& frameInfo, vle::GlobalUbo& ubo) override;
	void render(vle::FrameInfo& frameInfo) override;

private:
	void createPipeline(VkRenderPass renderPass) override;
};

#endif // GSPLATS_RENDER_SYSTEM_H