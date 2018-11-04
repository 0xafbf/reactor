#pragma once

#include "types.h"
#include "vulkan/vulkan_core.h"
#include "rmath.h"

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
	
	array<VkBuffer> vertexBuffers;
	
	rPrimitive(rEngine* inEngine, string inVertPath, string inFragPath);
	~rPrimitive();
};

void rPrimitiveDraw(rPrimitive* primitive, VkCommandBuffer buffer);


struct vert_data
{
	vec3 pos;
	vec3 color;
};

struct rScene
{
	array<rPrimitive*> primitives;
};

void rSceneDraw(rScene* scene, VkCommandBuffer buffer);