
#include "engine.h"
#include <iostream>

int main() {

	rEngine engine;
	engine.name = "My Greatest App";

	rStartEngine(&engine);

	rWindow window;
	window.width = 800;
	window.height = 600;
	window.name = "My Greatest Window";
	rCreateWindow(&engine, &window);
	
	rCreatePipeline(&engine, &window, "../../shaders/vert.spv", "../../shaders/frag.spv");
	
	rCreateFramebuffers(&engine, &window);
	
	VkCommandPool commandPool;
	rCreateCommandPool(&engine, &window, &commandPool);
	
	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;
	
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	VK_CHECK(vkCreateSemaphore(engine.device, &semaphoreInfo, nullptr, &imageAvailableSemaphore));
	VK_CHECK(vkCreateSemaphore(engine.device, &semaphoreInfo, nullptr, &renderFinishedSemaphore));
	
	while (!glfwWindowShouldClose(window.glfwWindow)) {
		glfwPollEvents();
		u32 imageIndex;
		VkResult nextImageResult = vkAcquireNextImageKHR(engine.device, window.swapchain, -1, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
		if (nextImageResult == VK_ERROR_OUT_OF_DATE_KHR)
		{
			std::cout << "vk out of date" << std::endl;
		}
		else if (nextImageResult == VK_SUBOPTIMAL_KHR)
		{
			std::cout << "vk suboptimal" << std::endl;
		}
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		VkPipelineStageFlags pipelineFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &imageAvailableSemaphore;
		submitInfo.pWaitDstStageMask = &pipelineFlags;
		
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &engine.commandBuffers[imageIndex];
		
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &renderFinishedSemaphore;
		
		VkResult submit = vkQueueSubmit(engine.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &renderFinishedSemaphore;
		
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &window.swapchain;
		presentInfo.pImageIndices = &imageIndex;
		
		VkResult present = vkQueuePresentKHR(engine.presentQueue, &presentInfo);
		
		vkQueueWaitIdle(engine.presentQueue);
	}
	
	vkDeviceWaitIdle(engine.device);
	vkDestroyCommandPool(engine.device, commandPool, nullptr);
	vkDestroySemaphore(engine.device, imageAvailableSemaphore, nullptr);
	vkDestroySemaphore(engine.device, renderFinishedSemaphore, nullptr);
	
	rDestroyEngine(&engine);
	
	return EXIT_SUCCESS;
}

