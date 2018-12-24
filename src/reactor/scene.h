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
	array<VkDescriptorSet> descriptorSets;

	u32 indexCount;
	VkBuffer indexBuffer;
	array<VkBuffer> vertexBuffers;

};

rGraphicsPipeline rPipeline(rEngine& inEngine, string inPath);

rGraphicsPipeline rPipeline(rEngine& inEngine, string inPath, rGeometry& geometry);

void rPipelineDraw(rGraphicsPipeline* pipeline, VkCommandBuffer buffer);
void rPipelineSetGeometry(rGraphicsPipeline& pipeline, rGeometry& geometry);


struct rScene
{
	array<rGraphicsPipeline*> primitives;
};

void rSceneDraw(rScene* scene, VkCommandBuffer buffer);
