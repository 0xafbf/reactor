	
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
	
	rEngine(string inName);
	~rEngine();
};

/**
 * This starts the engine, for now is just starting the vulkan instance and stuff
 */
void rEngineStart(rEngine* engineInst);
void rEngineDestroy(rEngine* engineInst);

void rCreateCommandPool(rEngine* engine, rWindow* window, VkCommandPool* commandPool);

bool rEngineShouldTick(rEngine* engine);
void rEngineTick(rEngine* engine);
void rEngineMainLoop(rEngine* engine);