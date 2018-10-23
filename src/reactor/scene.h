#pragma once

#include "types.h"
#include "vulkan/vulkan_core.h"

struct rEngine;

struct rPrimitive
{
	string vertPath;
	string fragPath;
	rEngine* engine;
	
	array<char> vertShader;
	array<char> fragShader;
	
	VkShaderModule vertModule;
	VkShaderModule fragModule;
	
	VkPipeline pipeline;
	
	rPrimitive(rEngine* inEngine, string inVertPath, string inFragPath);
	~rPrimitive();
};

void rPrimitiveDraw(rPrimitive* primitive, VkCommandBuffer buffer);



struct rScene
{
	array<rPrimitive*> primitives;
};

void rSceneDraw(rScene* scene, VkCommandBuffer buffer);