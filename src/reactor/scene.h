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
	string vertPath;
	string fragPath;
	rEngine* engine;
	
	array<char> vertShader;
	array<char> fragShader;
	
	VkShaderModule vertModule;
	VkShaderModule fragModule;
	
	VkPipeline pipeline;
	
	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout layout;
	array<VkDescriptorSet> descriptorSets;

	u32 indexCount;
	VkBuffer indexBuffer;
	array<VkBuffer> vertexBuffers;

};

rGraphicsPipeline rPipeline(rEngine& inEngine, string inVertPath, string inFragPath);

rGraphicsPipeline rPipeline(rEngine& inEngine, string inVertPath, string inFragPath, rGeometry& geometry);

void rPipelineDraw(rGraphicsPipeline* pipeline, VkCommandBuffer buffer);


struct rScene
{
	array<rGraphicsPipeline*> primitives;
};

void rSceneDraw(rScene* scene, VkCommandBuffer buffer);
