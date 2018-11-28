#pragma once

#include "types.h"
#include "vulkan/vulkan_core.h"
#include "rmath.h"

struct rEngine;

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

	
	rGraphicsPipeline(rEngine* inEngine, string inVertPath, string inFragPath);
	~rGraphicsPipeline();
};

struct rPrimitive {
	u32 indexCount;
	VkBuffer indexBuffer;
	array<VkBuffer> vertexBuffers;
	array<VkBuffer> uniformBuffers;

	rGraphicsPipeline* pipeline;
};

void rPrimitiveDraw(rPrimitive* primitive, rGraphicsPipeline* pipeline, VkCommandBuffer buffer);


struct rScene
{
	array<rPrimitive*> primitives;
};

void rSceneDraw(rScene* scene, VkCommandBuffer buffer);
