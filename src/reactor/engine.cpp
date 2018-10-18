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

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

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
	appInfo.pApplicationName = engineInst->name;
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

bool retrieveDeviceCapabilities(VkPhysicalDevice physicalDevice, rWindow* dumbWindow, QueueFamilyIndices* indices)
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

	for (u32 idx = 0; idx < queueFamilyCount; ++idx) {
		const auto& queueFamily = queueFamilies[idx];
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, idx, dumbWindow->surface, &presentSupport);
		if (queueFamily.queueCount > 0 && presentSupport) {
			indices->presentFamily = idx;
			break;
		}
	}
	
	if (indices->presentFamily == -1) return false;
	
	
	return true;
}

void initWindow(rEngine* engine, rWindow* window)
{
	window->engine = engine;
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	//glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
	//glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
	window->glfwWindow = glfwCreateWindow(window->width, window->height, window->name, nullptr, nullptr);

	VK_CHECK (glfwCreateWindowSurface(engine->instance, window->glfwWindow, nullptr, &window->surface));
	engine->windows.push_back(window);
}

void createDevice(rEngine* engineInst) {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(engineInst->instance, &deviceCount, nullptr);
	assert(deviceCount != 0);
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(engineInst->instance, &deviceCount, devices.data());
	
	rWindow dumbWindow{ "dumb_window", 1, 1 };
	
	initWindow(engineInst, &dumbWindow);
	for (const auto& device : devices) {
		if (retrieveDeviceCapabilities(device, &dumbWindow, &engineInst->indices))
		{
			engineInst->physicalDevice = device;
			break;
		}
	}
	rDestroyWindow(&dumbWindow);

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

void rCreateWindow(rEngine* engine, rWindow* window)
{
	initWindow(engine, window);
	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(engine->physicalDevice, window->surface, &capabilities);
	u32 wantedImageCount = capabilities.minImageCount;
	VkBool32 bSupported;
	vkGetPhysicalDeviceSurfaceSupportKHR(engine->physicalDevice, engine->indices.presentFamily, window->surface, &bSupported);

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

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = window->surface;
	createInfo.imageExtent = capabilities.currentExtent;
	createInfo.minImageCount = wantedImageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = capabilities.currentExtent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = bestMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	VK_CHECK(vkCreateSwapchainKHR(engine->device, &createInfo, nullptr, &window->swapchain));

	u32 imageCount = -1;
	vkGetSwapchainImagesKHR(engine->device, window->swapchain, &imageCount, nullptr);
	window->swapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(engine->device, window->swapchain, &imageCount, window->swapchainImages.data());

	window->swapchainImageViews.resize(imageCount);
	for (u32 idx = 0; idx < imageCount; ++idx)
	{
		VkImage image = window->swapchainImages[idx];
		VkImageViewCreateInfo imageViewCI = {};
		imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCI.image = window->swapchainImages[idx];
		imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCI.format = surfaceFormat.format;
		imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCI.subresourceRange.baseMipLevel = 0;
		imageViewCI.subresourceRange.levelCount = 1;
		imageViewCI.subresourceRange.baseArrayLayer = 0;
		imageViewCI.subresourceRange.layerCount = 1;


		VkImageView imageView;
		VK_CHECK(vkCreateImageView(engine->device, &imageViewCI, nullptr, &imageView));

		window->swapchainImageViews[idx] = imageView;
	}

	std::cout << "Hello";
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
	if (window->swapchain) vkDestroySwapchainKHR(window->engine->device, window->swapchain, nullptr);
	vkDestroySurfaceKHR(window->engine->instance, window->surface, nullptr);
	glfwDestroyWindow(window->glfwWindow);
	auto& vec = window->engine->windows;
	vec.erase(std::remove(vec.begin(), vec.end(), window), vec.end());
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

void rCreatePipeline(rEngine* engine, rWindow* window, std::string vertPath, std::string fragPath)
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
	
	VK_CHECK(vkCreateRenderPass(engine->device, &renderPassInfo, nullptr, &engine->renderPass));
	
	
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
	pipelineCreateInfo.renderPass = engine->renderPass;
	pipelineCreateInfo.subpass = 0;
	
	VK_CHECK(vkCreateGraphicsPipelines(engine->device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &window->graphicsPipeline));
	
	vkDestroyShaderModule(engine->device, vertModule, nullptr);
	vkDestroyShaderModule(engine->device, fragModule, nullptr);
}

void rCreateFramebuffers(rEngine* engine, rWindow* window)
{
	window->swapchainFramebuffers.resize(window->swapchainImageViews.size());
	
	for (u32 idx = 0; idx < window->swapchainImageViews.size(); ++idx)
	{
		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = engine->renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &window->swapchainImageViews[idx];
		framebufferInfo.width = window->width;
		framebufferInfo.height = window->height;
		framebufferInfo.layers = 1;
		
		VkFramebuffer framebuffer;
		VK_CHECK(vkCreateFramebuffer(engine->device, &framebufferInfo, nullptr, &framebuffer));
		window->swapchainFramebuffers[idx] = framebuffer;
	}
	
}

void rCreateCommandPool(rEngine* engine, rWindow* window, VkCommandPool* commandPool)
{
	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = engine->indices.graphicsFamily;
	VK_CHECK(vkCreateCommandPool(engine->device, &poolInfo, nullptr, commandPool));
	
	engine->commandBuffers.resize(window->swapchainFramebuffers.size());
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = *commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (u32)engine->commandBuffers.size();
	VK_CHECK(vkAllocateCommandBuffers(engine->device, &allocInfo, engine->commandBuffers.data()));
	
	for (u32 idx = 0; idx < engine->commandBuffers.size(); ++idx	)
	{
		VkCommandBuffer buffer = engine->commandBuffers[idx];
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		
		VK_CHECK(vkBeginCommandBuffer(buffer, &beginInfo));
		
		
		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = engine->renderPass;
		renderPassInfo.framebuffer = window->swapchainFramebuffers[idx];
		renderPassInfo.renderArea.offset = { 0,0 };
		renderPassInfo.renderArea.extent = { window->width, window->height };
		VkClearValue clearValue = {};
		clearValue.color = { 0.0f, 0.0f, 0.0f, 1.0f };
		
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearValue;
		
		vkCmdBeginRenderPass(buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		
		vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, window->graphicsPipeline);
		vkCmdDraw(buffer, 3, 1, 0, 0);
		vkCmdEndRenderPass(buffer);
		VK_CHECK(vkEndCommandBuffer(buffer));
		
	}
	
	
}