
#include "engine.h"
#include "window.h"
#include <iostream>

int main()
{
	rEngine _engine("My Great App");
	rStartEngine(&_engine);

	rWindow _window("My Greatest Window", 800, 600); //name, size
	rCreateWindow(&_engine, &_window);
	
	rEngine* engine = &_engine;
	rWindow* window = &_window;
	
	VkPipeline pipeline;
	rCreatePipeline(&_engine, &_window, "../shaders/vert.spv", "../shaders/frag.spv", window->renderPass, &pipeline);
	
	VkCommandPool commandPool;
	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = engine->indices.graphicsFamily;
	VK_CHECK(vkCreateCommandPool(engine->device, &poolInfo, nullptr, &commandPool));
	
	window->commandBuffers.resize(window->swapchainFramebuffers.size());
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (u32)window->commandBuffers.size();
	VK_CHECK(vkAllocateCommandBuffers(engine->device, &allocInfo, window->commandBuffers.data()));
	
	for (u32 idx = 0; idx < window->commandBuffers.size(); ++idx	)
	{
		VkCommandBuffer buffer = window->commandBuffers[idx];
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		
		VK_CHECK(vkBeginCommandBuffer(buffer, &beginInfo));
		
		
		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = window->renderPass;
		renderPassInfo.framebuffer = window->swapchainFramebuffers[idx];
		renderPassInfo.renderArea.offset = { 0,0 };
		renderPassInfo.renderArea.extent = { window->width, window->height };
		VkClearValue clearValue = {};
		clearValue.color = { 0.0f, 0.0f, 0.0f, 1.0f };
		
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearValue;
		
		vkCmdBeginRenderPass(buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		
		vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		vkCmdDraw(buffer, 3, 1, 0, 0);
		vkCmdEndRenderPass(buffer);
		VK_CHECK(vkEndCommandBuffer(buffer));
		
	}
	
	while (rEngineShouldTick(engine))
	{
		vkQueueWaitIdle(_engine.presentQueue);
		glfwPollEvents();


		for (rWindow* window : engine->windows)
		{
			if (glfwWindowShouldClose(window->glfwWindow))
			{
				rDestroyWindow(window);
				continue;
			}
		}
		u32 numWindows = u32(engine->windows.size());

		std::vector<u32> indices;
		indices.reserve(numWindows);
		std::vector<VkSwapchainKHR> swapchains;
		swapchains.reserve(numWindows);

		std::vector<VkSemaphore> windowSemaphores;
		windowSemaphores.reserve(numWindows);

		for (u32 idx = 0; idx < numWindows; ++idx)
		{
			rWindow* window = engine->windows[idx];
			u32 imageIndex;
			VkResult nextImageResult = vkAcquireNextImageKHR(engine->device, window->swapchain, -1, window->imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
			if (nextImageResult == VK_ERROR_OUT_OF_DATE_KHR)
			{
				std::cout << "vk out of date" << std::endl;
				//rWindowRefresh(window);
				//continue;
			}
			else if (nextImageResult == VK_SUBOPTIMAL_KHR)
			{
				std::cout << "vk suboptimal" << std::endl;
				// it is still considered valid, so continue
			}

			// swapchains here, as window swapchain may change in rWindowRefresh
			//swapchains[idx] = window->swapchain;
			swapchains.push_back(window->swapchain);
			indices.push_back(imageIndex);
			windowSemaphores.push_back(window->renderFinishedSemaphore);

			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			VkPipelineStageFlags pipelineFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = &window->imageAvailableSemaphore;
			submitInfo.pWaitDstStageMask = &pipelineFlags;

			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &window->commandBuffers[imageIndex];

			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = &window->renderFinishedSemaphore;

			VkResult submit = vkQueueSubmit(engine->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		}
		if (swapchains.size() > 0)
		{
			VkPresentInfoKHR presentInfo = {};
			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			presentInfo.waitSemaphoreCount = u32(windowSemaphores.size());
			presentInfo.pWaitSemaphores = windowSemaphores.data();

			presentInfo.swapchainCount = u32(swapchains.size());
			presentInfo.pSwapchains = swapchains.data();
			presentInfo.pImageIndices = indices.data();

			VkResult present = vkQueuePresentKHR(engine->presentQueue, &presentInfo);

		}
	}
	
	vkDeviceWaitIdle(_engine.device);
	vkDestroyCommandPool(_engine.device, commandPool, nullptr);
	
	rDestroyEngine(&_engine);
	string input;
	std::cin >> input;
	
	return EXIT_SUCCESS;
}

