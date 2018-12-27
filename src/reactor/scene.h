#pragma once

#include "types.h"
#include "vulkan/vulkan_core.h"
#include "rmath.h"
#include "buffer.h"

struct rEngine;
struct rBuffer;
struct rGeometry;


struct rGraphicsPipeline
{
	rEngine* engine;
	
	VkShaderModule vertModule;
	VkShaderModule fragModule;
	
	VkPipeline pipeline;
	
	array<VkDescriptorSetLayout> descriptor_set_layouts;
	VkPipelineLayout layout;

};

struct rState {
	rGraphicsPipeline* pipeline;
	rGeometry* geometry;
	array<VkDescriptorSet> descriptor_sets;
};

rGraphicsPipeline rPipeline(rEngine& inEngine, string inPath);

void rPipelineDraw(rState* state, VkCommandBuffer buffer);


struct rScene
{
	array<rState*> primitives;
};

void rSceneDraw(rScene* scene, VkCommandBuffer buffer);
