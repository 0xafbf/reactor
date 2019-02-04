#pragma once

#include "vulkan/vulkan_core.h"
#include "engine.h"

struct rGraphicsPipeline
{
	rEngine* engine;
	
	VkPipeline pipeline;
	
	array<VkDescriptorSetLayout> descriptor_set_layouts;
	VkPipelineLayout layout;
	VkPipelineRasterizationStateCreateInfo rasterizer;
};


rGraphicsPipeline rPipeline(rEngine& inEngine, string inPath);
