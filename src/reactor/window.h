
#pragma once


#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"

#include "types.h"
#include <vector>

struct rEngine;

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
	VkRenderPass renderPass;

	std::vector<VkImage> swapchainImages;
	std::vector<VkImageView> swapchainImageViews;
	std::vector<VkFramebuffer> swapchainFramebuffers;
	VkSemaphore renderFinishedSemaphore;
	VkSemaphore imageAvailableSemaphore;
	std::vector<VkCommandBuffer> commandBuffers;
	
	rWindow(string inName, u32 inWidth, u32 inHeight)
		: name(inName)
		, width(inWidth)
		, height(inHeight)
		, swapchain(VK_NULL_HANDLE)
	{
		
	}
};

void rCreateWindow(rEngine* engine, rWindow* window);
void rDestroyWindow(rWindow* window);
void rWindowRefresh(rWindow* window);

void rWindowRender(rWindow* window);

