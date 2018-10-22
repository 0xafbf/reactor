#include "window.h"
#include "engine.h"
#include <vector>
#include <assert.h>
#include <iostream>

#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"

#include "types.h"
#include <set>
#include <algorithm>
#include <iostream>
#include <iosfwd>
#include <fstream>

void glfwResizeCallback(GLFWwindow* glfwWindow, i32 width, i32 height)
{
	rWindow* window = (rWindow*) glfwGetWindowUserPointer(glfwWindow);
	rWindowRefresh(window);
}

void rCreateWindow(rEngine* engine, rWindow* window)
{
	window->engine = engine;
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	//glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
	//glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
	window->glfwWindow = glfwCreateWindow(window->width, window->height, window->name.c_str(), nullptr, nullptr);
	
	glfwSetFramebufferSizeCallback(window->glfwWindow, glfwResizeCallback);
	glfwSetWindowUserPointer(window->glfwWindow, window);

	VK_CHECK (glfwCreateWindowSurface(engine->instance, window->glfwWindow, nullptr, &window->surface));
	engine->windows.push_back(window);
	
	VkBool32 bSupported;
	vkGetPhysicalDeviceSurfaceSupportKHR(engine->physicalDevice, engine->indices.presentFamily, window->surface, &bSupported);
	assert(bSupported);
	
	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(engine->physicalDevice, window->surface, &capabilities);
	window->wantedImageCount = capabilities.minImageCount;
	u32 formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(engine->physicalDevice, window->surface, &formatCount, nullptr);
	std::vector<VkSurfaceFormatKHR> formats(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(engine->physicalDevice, window->surface, &formatCount, formats.data());

	VkSurfaceFormatKHR surfaceFormat = { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	/*
	if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
		return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}*/
	// let's just try different settings and explore
	window->surfaceFormat = surfaceFormat;


	u32 presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(engine->physicalDevice, window->surface, &presentModeCount, nullptr);
	std::vector<VkPresentModeKHR> presentModes(presentModeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(engine->physicalDevice, window->surface, &presentModeCount, presentModes.data());
	VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

	for (const auto& availablePresentMode : presentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			bestMode = availablePresentMode;
		}
		else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
			bestMode = availablePresentMode;
		}
	}
	window->presentMode = bestMode;

	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = VK_FORMAT_B8G8R8A8_UNORM;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	
	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	
	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;
	
	VK_CHECK(vkCreateRenderPass(engine->device, &renderPassInfo, nullptr, &window->renderPass));
	
	rWindowRefresh(window);
	
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	
	vkCreateSemaphore(engine->device, &semaphoreInfo, nullptr, &window->renderFinishedSemaphore);
	vkCreateSemaphore(engine->device, &semaphoreInfo, nullptr, &window->imageAvailableSemaphore);
}
void rDestroyWindow(rWindow* window)
{
//		for (auto imageView : swapChainImageViews) {
//			vkDestroyImageView(device, imageView, nullptr);
//		}
  	//vkDestroySwapchainKHR(device, swapChain, nullptr);
  	for (VkFramebuffer framebuffer : window->swapchainFramebuffers)
  	{
  		vkDestroyFramebuffer(window->engine->device, framebuffer, nullptr);
  	}
  	for (VkImageView imageView : window->swapchainImageViews)
  	{
  		vkDestroyImageView(window->engine->device, imageView, nullptr);
  	}
  	if (window->renderFinishedSemaphore) vkDestroySemaphore(window->engine->device, window->renderFinishedSemaphore, nullptr);
  	if (window->imageAvailableSemaphore) vkDestroySemaphore(window->engine->device, window->imageAvailableSemaphore, nullptr);
	if (window->renderPass) vkDestroyRenderPass(window->engine->device, window->renderPass, nullptr);
  	if (window->swapchain) vkDestroySwapchainKHR(window->engine->device, window->swapchain, nullptr);
  	vkDestroySurfaceKHR(window->engine->instance, window->surface, nullptr);
  	glfwDestroyWindow(window->glfwWindow);
  	auto& vec = window->engine->windows;
  	vec.erase(std::remove(vec.begin(), vec.end(), window), vec.end());
}

void rWindowRefresh(rWindow* window)
{
	rEngine* engine = window->engine;
	
	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(engine->physicalDevice, window->surface, &capabilities);

	window->width = capabilities.currentExtent.width;
	window->height = capabilities.currentExtent.height;
	
	VkSwapchainKHR newSwapchain;
	
	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = window->surface;
	createInfo.imageExtent = capabilities.currentExtent;
	createInfo.minImageCount = window->wantedImageCount;
	createInfo.imageFormat = window->surfaceFormat.format;
	createInfo.imageColorSpace = window->surfaceFormat.colorSpace;
	createInfo.imageExtent = capabilities.currentExtent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = window->presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = window->swapchain;;

	VK_CHECK(vkCreateSwapchainKHR(engine->device, &createInfo, nullptr, &newSwapchain));
	
	u32 imageCount = -1;
	vkGetSwapchainImagesKHR(engine->device, newSwapchain, &imageCount, nullptr);
	window->swapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(engine->device, newSwapchain, &imageCount, window->swapchainImages.data());
	
	window->swapchainImageViews.resize(imageCount);
	window->swapchainFramebuffers.resize(imageCount);
	
	for (u32 idx = 0; idx < imageCount; ++idx)
	{
		VkImageViewCreateInfo imageViewCI = {};
		imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCI.image = window->swapchainImages[idx];
		imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCI.format = window->surfaceFormat.format;
		imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCI.subresourceRange.baseMipLevel = 0;
		imageViewCI.subresourceRange.levelCount = 1;
		imageViewCI.subresourceRange.baseArrayLayer = 0;
		imageViewCI.subresourceRange.layerCount = 1;
		
		VK_CHECK(vkCreateImageView(engine->device, &imageViewCI, nullptr, &window->swapchainImageViews[idx]));

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = window->renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &window->swapchainImageViews[idx];
		framebufferInfo.width = window->width;
		framebufferInfo.height = window->height;
		framebufferInfo.layers = 1;
		
		VK_CHECK(vkCreateFramebuffer(engine->device, &framebufferInfo, nullptr, &window->swapchainFramebuffers[idx]));
	}
	
	
	vkDestroySwapchainKHR(window->engine->device, window->swapchain, nullptr);
	window->swapchain = newSwapchain;
	
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	
	//vkCreateSemaphore(engine->device, &semaphoreInfo, nullptr, &window->imageAvailableSemaphore);

}

void rWindowRender(rWindow* window)
{
	
}