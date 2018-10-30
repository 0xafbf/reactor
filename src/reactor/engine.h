	
#pragma once

#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"

#include "types.h"
#include <vector>


struct QueueFamilyIndices
{
	u32 graphicsFamily = -1;
	u32 presentFamily = -1;
};


struct rWindow;

struct rEngine
{
	string name;
	
	// vulkan stuff, maybe better move to a vulkan struct?
	VkInstance instance;
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	QueueFamilyIndices indices;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	std::vector<rWindow*> windows;
	
	VkRenderPass primitivesRenderPass;
	VkCommandPool commandPool;
	
	rEngine(string inName);
	~rEngine();
	VkDescriptorPool descriptorPool;
};

/**
 * This starts the engine, for now is just starting the vulkan instance and stuff
 */
void rEngineStart(rEngine* engineInst);
void rEngineDestroy(rEngine* engineInst);

bool rEngineShouldTick(rEngine* engine);
void rEngineTick(rEngine* engine);
void rEngineMainLoop(rEngine* engine);