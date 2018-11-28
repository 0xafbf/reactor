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
#include <imgui.h>

#include "scene.h"
#include "examples/imgui_impl_glfw.h"
#include "examples/imgui_impl_vulkan.h"

void glfwResizeCallback(GLFWwindow* glfwWindow, i32 width, i32 height)
{
	rWindow* window = (rWindow*) glfwGetWindowUserPointer(glfwWindow);
	
	vkDeviceWaitIdle(window->engine->device);
	rWindowDestroySwapChain(window);
	rWindowCreateSwapChain(window);
	
	// this would be nice, but it requires more setup
	// rWindowRender(window);
}

void rCreateWindow(rWindow* window)
{
	
	rEngine* engine = window->engine;
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

	
	rWindowCreateSwapChain(window);
	
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	
	vkCreateSemaphore(engine->device, &semaphoreInfo, nullptr, &window->renderFinishedSemaphore);
	vkCreateSemaphore(engine->device, &semaphoreInfo, nullptr, &window->imageAvailableSemaphore);
	
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = engine->commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;
	VK_CHECK(vkAllocateCommandBuffers(engine->device, &allocInfo, &window->commandBuffer));
	
	ImGui_ImplGlfw_InitForVulkan(window->glfwWindow, true);
	
	
}
void rDestroyWindow(rWindow* window)
{
	rWindowDestroySwapChain(window);
	if (window->renderFinishedSemaphore) vkDestroySemaphore(window->engine->device, window->renderFinishedSemaphore, nullptr);
	if (window->imageAvailableSemaphore) vkDestroySemaphore(window->engine->device, window->imageAvailableSemaphore, nullptr);
	vkDestroySurfaceKHR(window->engine->instance, window->surface, nullptr);
	glfwDestroyWindow(window->glfwWindow);
	auto& vec = window->engine->windows;
	vec.erase(std::remove(vec.begin(), vec.end(), window), vec.end());
}

void rWindowCreateSwapChain(rWindow* window)
{
	rEngine* engine = window->engine;
	
	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(engine->physicalDevice, window->surface, &capabilities);

	window->width = capabilities.currentExtent.width;
	window->height = capabilities.currentExtent.height;
	
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
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	VK_CHECK(vkCreateSwapchainKHR(engine->device, &createInfo, nullptr, &window->swapchain));
	
	u32 imageCount = -1;
	vkGetSwapchainImagesKHR(engine->device, window->swapchain, &imageCount, nullptr);
	window->swapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(engine->device, window->swapchain, &imageCount, window->swapchainImages.data());
	
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
		framebufferInfo.renderPass = engine->primitivesRenderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &window->swapchainImageViews[idx];
		framebufferInfo.width = window->width;
		framebufferInfo.height = window->height;
		framebufferInfo.layers = 1;
		
		VK_CHECK(vkCreateFramebuffer(engine->device, &framebufferInfo, nullptr, &window->swapchainFramebuffers[idx]));
	}
	
	
}

void rWindowDestroySwapChain(rWindow* window)
{
	VkDevice device = window->engine->device;

	vkDestroySwapchainKHR(device, window->swapchain, nullptr);

	for (VkImageView imageView : window->swapchainImageViews)
	{
		vkDestroyImageView(device, imageView, nullptr);
	}
	
	for (VkFramebuffer framebuffer : window->swapchainFramebuffers)
	{
		vkDestroyFramebuffer(device, framebuffer, nullptr);
	}
}

bool rWindowRender(rWindow* window)
{
	VkResult nextImageResult = vkAcquireNextImageKHR(window->engine->device, window->swapchain, -1, window->imageAvailableSemaphore, VK_NULL_HANDLE, &window->imageIndex);
	if (nextImageResult == VK_ERROR_OUT_OF_DATE_KHR)
	{
		std::cout << "vk out of date" << std::endl;
		//rWindowRefresh(window);
		//continue;
		return false;
	}
	else if (nextImageResult == VK_SUBOPTIMAL_KHR)
	{
		std::cout << "vk suboptimal" << std::endl;
		// it is still considered valid, so continue
	}
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	
	VkCommandBuffer commandBuffer = window->commandBuffer;
	VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));
	
	
	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = window->engine->primitivesRenderPass;
	renderPassInfo.framebuffer = window->swapchainFramebuffers[window->imageIndex];
	renderPassInfo.renderArea.offset = { 0,0 };
	renderPassInfo.renderArea.extent = { window->width, window->height };
	VkClearValue clearValue = {};
	clearValue.color = { 0.0f, 0.0f, 0.0f, 1.0f };
	
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearValue;
	
	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	

	// Update dynamic viewport state
	VkViewport viewport = {};
	viewport.height = float(window->height);
	viewport.width = float(window->width);
	viewport.minDepth = (float) 0.0f;
	viewport.maxDepth = (float) 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	// Update dynamic scissor state
	VkRect2D scissor = {};
	scissor.extent.width = window->width;
	scissor.extent.height = window->height;
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	
	if (window->scene)
	{
		rSceneDraw(window->scene, commandBuffer);
		
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
	}
	vkCmdEndRenderPass(commandBuffer);
	VK_CHECK(vkEndCommandBuffer(commandBuffer));

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	VkPipelineStageFlags pipelineFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &window->imageAvailableSemaphore;
	submitInfo.pWaitDstStageMask = &pipelineFlags;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &window->renderFinishedSemaphore;

	VkResult submit = vkQueueSubmit(window->engine->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	
	return true;
}

rWindow::rWindow(rEngine* inEngine, string inName, u32 inWidth, u32 inHeight) : engine(inEngine)
, name(inName)
, width(inWidth)
, height(inHeight)
, swapchain(VK_NULL_HANDLE)
, scene(nullptr)
{
	rCreateWindow(this);
}
