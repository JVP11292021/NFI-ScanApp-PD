#include "SplatRenderSystem.hpp"

SplatRenderSystem::SplatRenderSystem(vle::EngineDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) 
	: Base(device, globalSetLayout)
{
	this->createPipeline(renderPass);
}

void SplatRenderSystem::update(vle::FrameInfo& frameInfo, vle::GlobalUbo& ubo) {

}

void SplatRenderSystem::render(vle::FrameInfo& frameInfo) {

}

void SplatRenderSystem::createPipeline(VkRenderPass renderPass) {

}
