#include "engine.h"

#include <vector>
#include <assert.h>
#include <iostream>
#include <set>
#include <algorithm>
#include <iostream>
#include <iosfwd>
#include <fstream>

#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"

#include "types.h"
#include "window.h"

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

//#define DEBUG_VK_CALLBACK

std::vector<const char*> getValidationLayers()
{
	if (!enableValidationLayers ) {
		return std::vector<const char*>();
	}
	std::vector<const char*> validationLayers = {
		"VK_LAYER_LUNARG_standard_validation"
	};
	
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers) {
		bool layerFound = false;
		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}
		assert(layerFound);
	}
	return validationLayers;
}

std::vector<const char*> getRequiredExtensions() {
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
	if (enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
	return extensions;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pCallback) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	assert(func);
	return func(instance, pCreateInfo, pAllocator, pCallback);
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT callback, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	assert(func);
	func(instance, callback, pAllocator);
}

VkDebugUtilsMessengerEXT callbackHandle;
VkBool32 debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl << std::endl;
	// uncomment for breaking in error
#ifdef DEBUG_VK_CALLBACK
	assert(messageSeverity < VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT);
#endif
	return VK_FALSE;
};
void setupDebugCallback(VkInstance instance) {
	if (!enableValidationLayers) return;
	
	VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = (PFN_vkDebugUtilsMessengerCallbackEXT)debugCallback;

	VK_CHECK(CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &callbackHandle));
}

void createDevice(rEngine* engineInst);

void rStartEngine(rEngine* engineInst)
{
	// we need to do this first as getRequiredExtensions relies on it
	glfwInit();
	
	// Set up the vulkan instance
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = engineInst->name.c_str();
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "reactor Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	auto extensions = getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();
	createInfo.enabledLayerCount = 0;
	
	std::vector<const char*> validationLayers = getValidationLayers();
	createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
	createInfo.ppEnabledLayerNames = validationLayers.data();

	VK_CHECK(vkCreateInstance(&createInfo, nullptr, &engineInst->instance));

	//////
	setupDebugCallback(engineInst->instance);	
	
	createDevice(engineInst);
	
}

void rDestroyEngine(rEngine* engineInst)
{
	vkDeviceWaitIdle(engineInst->device);
	for (rWindow* window : engineInst->windows)
	{
		rDestroyWindow(window);
	}

	vkDestroyDevice(engineInst->device, nullptr);

	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(engineInst->instance, callbackHandle, nullptr);
	}

	vkDestroyInstance(engineInst->instance, nullptr);


	glfwTerminate();
		
}

bool retrieveDeviceCapabilities(VkPhysicalDevice physicalDevice, QueueFamilyIndices* indices)
{
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

	for (u32 idx = 0; idx < queueFamilyCount; ++idx) {
		const auto& queueFamily = queueFamilies[idx];
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices->graphicsFamily = idx;
			break;
		}
	}
	if (indices->graphicsFamily == -1) return false;
	
	indices->presentFamily = indices->graphicsFamily;
	
	return true;
}

void initWindow(rEngine* engine, rWindow* window)
{
}

void createDevice(rEngine* engineInst) {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(engineInst->instance, &deviceCount, nullptr);
	assert(deviceCount != 0);
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(engineInst->instance, &deviceCount, devices.data());
	
	for (const auto& device : devices) {
		if (retrieveDeviceCapabilities(device, &engineInst->indices))
		{
			engineInst->physicalDevice = device;
			break;
		}
	}

	assert(engineInst->physicalDevice);


	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(engineInst->physicalDevice, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(engineInst->physicalDevice, &queueFamilyCount, queueFamilies.data());

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set <u32> indicesSet = { engineInst->indices.graphicsFamily, engineInst->indices.presentFamily };
	for (u32 idx : indicesSet) {
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = idx;
		queueCreateInfo.queueCount = 1;
		float queuePriority = 1.0f;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	
	VkPhysicalDeviceFeatures deviceFeatures = {};
	createInfo.pEnabledFeatures = &deviceFeatures;
	
	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	std::vector<const char*> validationLayers = getValidationLayers();
	createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
	createInfo.ppEnabledLayerNames = validationLayers.data();

	VK_CHECK(vkCreateDevice(engineInst->physicalDevice, &createInfo, nullptr, &engineInst->device));
	
	vkGetDeviceQueue(engineInst->device, engineInst->indices.graphicsFamily, 0, &engineInst->graphicsQueue);
	vkGetDeviceQueue(engineInst->device, engineInst->indices.presentFamily, 0, &engineInst->presentQueue);
}


std::vector<char> loadFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);
	assert(file.is_open());
	size_t filesize = (size_t)file.tellg();
	std::vector<char> buffer(filesize);
	file.seekg(0);
	file.read(buffer.data(), filesize);
	file.close();
	return buffer;
}

void rCreatePipeline(rEngine* engine, rWindow* window, std::string vertPath, std::string fragPath, VkRenderPass renderPass, VkPipeline* pipelineOut)
{
	// load shaders
	auto vertShader = loadFile(vertPath);
	auto fragShader = loadFile(fragPath);

	VkShaderModuleCreateInfo vertInfo = {};
	vertInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	vertInfo.codeSize = vertShader.size();
	vertInfo.pCode = reinterpret_cast<const u32*>(vertShader.data());
	VkShaderModule vertModule;
	VK_CHECK(vkCreateShaderModule(engine->device, &vertInfo, nullptr, &vertModule));
	
	VkShaderModuleCreateInfo fragInfo = {};
	fragInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	fragInfo.codeSize = fragShader.size();
	fragInfo.pCode = reinterpret_cast<const u32*>(fragShader.data());
	VkShaderModule fragModule;
	VK_CHECK(vkCreateShaderModule(engine->device, &fragInfo, nullptr, &fragModule));
	
	VkPipelineShaderStageCreateInfo vertStageInfo = {};
	vertStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertStageInfo.module = vertModule;
	vertStageInfo.pName = "main";
	
	VkPipelineShaderStageCreateInfo fragStageInfo = {};
	fragStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragStageInfo.module = fragModule;
	fragStageInfo.pName = "main";
	
	VkPipelineShaderStageCreateInfo shaderStages[] = { vertStageInfo, fragStageInfo };
	
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;
	
	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.stageCount = 2;
	pipelineCreateInfo.pStages = shaderStages;
	
	pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
	pipelineCreateInfo.pInputAssemblyState = &inputAssembly;	
	
	
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)window->width;
	viewport.height = (float)window->height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	VkRect2D scissor = {};
	scissor.offset = { 0,0 };
	scissor.extent = { window->width, window->height };
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;
	
	pipelineCreateInfo.pViewportState = &viewportState;
	
	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE; 
	
	pipelineCreateInfo.pRasterizationState = &rasterizer;

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = false;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	pipelineCreateInfo.pMultisampleState = &multisampling;

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT |
		VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlendInfo = {};
	colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendInfo.logicOpEnable = false;
	colorBlendInfo.attachmentCount = 1;
	colorBlendInfo.pAttachments = &colorBlendAttachment;
	pipelineCreateInfo.pColorBlendState = &colorBlendInfo;


	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

	VkPipelineLayout pipelineLayout;
	VK_CHECK(vkCreatePipelineLayout(engine->device, &pipelineLayoutInfo, nullptr, &pipelineLayout));
	
	pipelineCreateInfo.layout = pipelineLayout;
	pipelineCreateInfo.renderPass = renderPass;
	pipelineCreateInfo.subpass = 0;
	
	VK_CHECK(vkCreateGraphicsPipelines(engine->device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, pipelineOut));
	
	vkDestroyShaderModule(engine->device, vertModule, nullptr);
	vkDestroyShaderModule(engine->device, fragModule, nullptr);
}


void rCreateCommandPool(rEngine* engine, rWindow* window, VkCommandPool* commandPool)
{
	
	
	
}

bool rEngineShouldTick(rEngine* engine)
{
	if (engine->windows.size() > 0)
	{
		return true;
	}
	return false;
}


void rTickEngine(rEngine* engine)
{
	glfwPollEvents();
	
	
	for (rWindow* window : engine->windows)
	{
		if (glfwWindowShouldClose(window->glfwWindow))
		{
			rDestroyWindow(window);
			return;
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
			rWindowRefresh(window);
			continue;
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
