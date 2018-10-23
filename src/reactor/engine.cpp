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

#define DEBUG_VK_CALLBACK

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
void createRenderPasses(rEngine* engine);

void rEngineStart(rEngine* engineInst)
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
	createRenderPasses(engineInst);
	
}

void rEngineDestroy(rEngine* engineInst)
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


void createRenderPasses(rEngine* engine)
{
	// load shaders
	
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
	
	VK_CHECK(vkCreateRenderPass(engine->device, &renderPassInfo, nullptr, &engine->primitivesRenderPass));
	
		
}



bool rEngineShouldTick(rEngine* engine)
{
	if (engine->windows.size() > 0)
	{
		return true;
	}
	return false;
}


void rEngineTick(rEngine* engine)
{
	vkQueueWaitIdle(engine->presentQueue);
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

		if (rWindowRender(window))
		{
			swapchains.push_back(window->swapchain);
			indices.push_back(window->imageIndex);
			windowSemaphores.push_back(window->renderFinishedSemaphore);
		}
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

void rEngineMainLoop(rEngine* engine)
{

	while (rEngineShouldTick(engine))
	{
		glfwPollEvents();
		rEngineTick(engine);
	}
}

rEngine::rEngine(string inName) : name(inName)
{
	rEngineStart(this);
}

rEngine::~rEngine()
{
	rEngineDestroy(this);
}
