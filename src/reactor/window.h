
#pragma once


#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"

#include "types.h"
#include <vector>

struct rEngine;
struct rScene;
struct rWindow;


struct rWindow
{
	string name;
	u32 width;
	u32 height;
	GLFWwindow* glfwWindow;
	VkSurfaceKHR surface;
	
	VkSurfaceFormatKHR surfaceFormat;
	VkPresentModeKHR presentMode;
	VkExtent2D extent;
	rEngine* engine; 
	
	u32 wantedImageCount;
	VkSwapchainKHR swapchain;

	std::vector<VkImage> swapchainImages;
	std::vector<VkImageView> swapchainImageViews;
	std::vector<VkFramebuffer> swapchainFramebuffers;
	VkSemaphore renderFinishedSemaphore;
	VkSemaphore imageAvailableSemaphore;
	
	rScene* scene;
	
	rWindow(rEngine* inEngine, string inName, u32 inWidth, u32 inHeight);
	VkCommandBuffer commandBuffer;
	VkCommandPool commandPool;
	u32 imageIndex;
};

void rCreateWindow(rWindow* window);
void rDestroyWindow(rWindow* window);
void rWindowRecreateSwapChain(rWindow* window);

bool rWindowRender(rWindow* window);

