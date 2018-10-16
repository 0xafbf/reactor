	
#pragma once

#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"

#include "types.h"
#include <vector>


struct rWindow;

struct rEngine
{
	char* name;
	
	// vulkan stuff, maybe better move to a vulkan struct?
	VkInstance instance;
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	std::vector<rWindow*> windows;
};

/**
 * This starts the engine, for now is just starting the vulkan instance and stuff
 */
void rStartEngine(rEngine* engineInst);
void rDestroyEngine(rEngine* engineInst);

struct rWindow
{
	char* name;
	i32 width;
	i32 height;
	GLFWwindow* glfwWindow;
	VkSurfaceKHR surface;
	
	VkSurfaceFormatKHR surfaceFormat;
	VkPresentModeKHR presentMode;
	VkExtent2D extent;
	rEngine* engine;
	VkSwapchainKHR swapchain;

	std::vector<VkImage> swapchainImages;
	std::vector<VkImageView> swapchainImageViews;
};

void rCreateWindow(rEngine* engine, rWindow* window);
void rDestroyWindow(rWindow* window);

void rCreatePipeline();