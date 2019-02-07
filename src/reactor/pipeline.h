#pragma once

#include "vulkan/vulkan_core.h"
#include "engine.h"

struct rShader {

	array<VkPipelineShaderStageCreateInfo> shader_stages;
	array<VkVertexInputBindingDescription> input_bindings;
	array<VkVertexInputAttributeDescription> input_attributes;
	VkShaderModule vert_module;
	VkShaderModule frag_module;

	struct SlangReflection* reflection;

	array<VkDescriptorSetLayout> descriptor_set_layouts;
	VkPipelineLayout layout;

};

struct rGraphicsPipeline
{
	rEngine* engine;
	rShader shader;
	VkPipeline pipeline;
	

	VkPipelineRasterizationStateCreateInfo rasterizer;
	VkPipelineInputAssemblyStateCreateInfo input_assembly;
};


rGraphicsPipeline rPipeline(rEngine& inEngine, string inPath);
void rPipelineUpdate(rGraphicsPipeline& pipeline);