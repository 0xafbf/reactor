	
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
	char* name;
	
	// vulkan stuff, maybe better move to a vulkan struct?
	VkInstance instance;
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	QueueFamilyIndices indices;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	std::vector<rWindow*> windows;
	VkRenderPass renderPass;
	std::vector<VkCommandBuffer> commandBuffers;
};

/**
 * This starts the engine, for now is just starting the vulkan instance and stuff
 */
void rStartEngine(rEngine* engineInst);
void rDestroyEngine(rEngine* engineInst);

struct rWindow
{
	char* name;
	u32 width;
	u32 height;
	GLFWwindow* glfwWindow;
	VkSurfaceKHR surface;
	
	VkSurfaceFormatKHR surfaceFormat;
	VkPresentModeKHR presentMode;
	VkExtent2D extent;
	rEngine* engine;
	VkSwapchainKHR swapchain;

	std::vector<VkImage> swapchainImages;
	std::vector<VkImageView> swapchainImageViews;
	std::vector<VkFramebuffer> swapchainFramebuffers;
	VkPipeline graphicsPipeline;
};

void rCreateWindow(rEngine* engine, rWindow* window);
void rDestroyWindow(rWindow* window);


void rCreatePipeline(rEngine* engine, rWindow* window, std::string vertPath, std::string fragPath );
void rCreateFramebuffers(rEngine* engine, rWindow* window);
void rCreateCommandPool(rEngine* engine, rWindow* window, VkCommandPool* commandPool);