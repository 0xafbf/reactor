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
#include "imgui.h"

#include "types.h"
#include "window.h"
#include "examples/imgui_impl_vulkan.h"
#include "../../deps/imgui/examples/imgui_impl_glfw.h"

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
	array<VkLayerProperties> availableLayers(layerCount);
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

void createDevice(rEngine& engine);
void createRenderPasses(rEngine* engine);

void rEngineStart(rEngine* engine)
{
	// we need to do this first as getRequiredExtensions relies on it
	glfwInit();
	
	// Set up the vulkan instance
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = engine->name.c_str();
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

	VK_CHECK(vkCreateInstance(&createInfo, nullptr, &engine->instance));

	//////
	setupDebugCallback(engine->instance);	
	
	createDevice(*engine);
	createRenderPasses(engine);
	
	VkDescriptorPoolSize pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};
	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
	pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;
	VK_CHECK(vkCreateDescriptorPool(engine->device, &pool_info, nullptr, &engine->descriptorPool));
	
	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = engine->indices.graphicsFamily;
	VK_CHECK(vkCreateCommandPool(engine->device, &poolInfo, nullptr, &engine->commandPool));
	
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = engine->instance;
	init_info.PhysicalDevice = engine->physicalDevice;
	init_info.Device = engine->device;
	init_info.QueueFamily = engine->indices.graphicsFamily;
	init_info.Queue = engine->graphicsQueue;
	//init_info.PipelineCache = g_PipelineCache;
	init_info.DescriptorPool = engine->descriptorPool;
	//init_info.Allocator = g_Allocator;
	//init_info.CheckVkResultFn = check_vk_result;
	ImGui_ImplVulkan_Init(&init_info, engine->primitivesRenderPass);
	
	ImGui::CreateContext();
    ImGui::StyleColorsDark();
    // Upload Fonts
    {
        // Use any command queue
        VkCommandPool command_pool =  engine->commandPool;
		
		VkCommandBufferAllocateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		bufferInfo.commandBufferCount = 1;
		bufferInfo.commandPool = command_pool;
		VkCommandBuffer command_buffer;
		vkAllocateCommandBuffers(engine->device, &bufferInfo, &command_buffer);

		VK_CHECK(vkResetCommandPool(engine->device, command_pool, 0));
        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		VK_CHECK(vkBeginCommandBuffer(command_buffer, &begin_info));

        ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

        VkSubmitInfo end_info = {};
        end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        end_info.commandBufferCount = 1;
        end_info.pCommandBuffers = &command_buffer;
		VK_CHECK(vkEndCommandBuffer(command_buffer));
		VK_CHECK(vkQueueSubmit(engine->graphicsQueue, 1, &end_info, VK_NULL_HANDLE));
		VK_CHECK(vkDeviceWaitIdle(engine->device));
        ImGui_ImplVulkan_InvalidateFontUploadObjects();
    }
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

u32 rEngineGetMemoryIdx(rEngine & engine, VkMemoryPropertyFlags flags)
{
	let& memProps = engine.memProperties;
	u32 mem_idx = -1;
	for (u32 idx = 0; idx < memProps.memoryTypeCount; ++idx)
	{
		let& memoryType = memProps.memoryTypes[idx];
		if ((memoryType.propertyFlags & flags) == flags) {
			mem_idx = idx;
		}
	}
	assert(mem_idx != -1);
	return mem_idx;
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

void createDevice(rEngine& engine) {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(engine.instance, &deviceCount, nullptr);
	assert(deviceCount != 0);
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(engine.instance, &deviceCount, devices.data());
	
	for (const auto& device : devices) {
		if (retrieveDeviceCapabilities(device, &engine.indices))
		{
			engine.physicalDevice = device;
			break;
		}
	}

	assert(engine.physicalDevice);


	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(engine.physicalDevice, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(engine.physicalDevice, &queueFamilyCount, queueFamilies.data());

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set <u32> indicesSet = { engine.indices.graphicsFamily, engine.indices.presentFamily };
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

	VK_CHECK(vkCreateDevice(engine.physicalDevice, &createInfo, nullptr, &engine.device));
	
	vkGetDeviceQueue(engine.device, engine.indices.graphicsFamily, 0, &engine.graphicsQueue);
	vkGetDeviceQueue(engine.device, engine.indices.presentFamily, 0, &engine.presentQueue);

	vkGetPhysicalDeviceMemoryProperties(engine.physicalDevice, &engine.memProperties);
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



bool rEngineShouldTick(rEngine& engine)
{
	if (engine.windows.size() > 0)
	{
		return true;
	}
	return false;
}


bool rEngineStartFrame(rEngine& engine)
{
	if (!rEngineShouldTick(engine)) return false;

	glfwPollEvents();
	// this should be handled better for each window I guess.
	ImGui_ImplGlfw_NewFrame();

	ImGui::NewFrame();

	return true;
}

void rEngineEndFrame(rEngine& inEngine)
{
	rEngine* engine = &inEngine;
	ImGui::Render();
	
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

void rEngineMainLoop(rEngine& engine)
{
	while (rEngineShouldTick(engine))
	{
		rEngineStartFrame(engine);
		rEngineEndFrame(engine);
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
