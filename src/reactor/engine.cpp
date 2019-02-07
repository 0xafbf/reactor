
#include "stb_image_write.h"

#include "engine.h"

#include <vector>
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
#include "examples/imgui_impl_glfw.h"

#include "reactor.h"

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
		CHECK(layerFound);
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
	CHECK(func);
	return func(instance, pCreateInfo, pAllocator, pCallback);
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT callback, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	CHECK(func);
	func(instance, callback, pAllocator);
}

VkDebugUtilsMessengerEXT callbackHandle;
VkBool32 debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
	auto level = rLogLevel::debug;
	switch (messageSeverity)
	{
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: level = rLogLevel::debug; break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: level = rLogLevel::info; break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: level = rLogLevel::warn; break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: level = rLogLevel::error; break;
	}
	LOG(level, pCallbackData->pMessage);
	//std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl << std::endl;
	// uncomment for breaking in error
#ifdef DEBUG_VK_CALLBACK
	CHECK(messageSeverity < VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT);
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
		let command_buffer = beginSingleTimeCommands(*engine);
		ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
		endSingleTimeCommands(*engine, command_buffer);
        ImGui_ImplVulkan_InvalidateFontUploadObjects();
    }

	
	auto pool_sizes_2 = array<VkDescriptorPoolSize>();
	pool_sizes_2.push_back(VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10});
	pool_sizes_2.push_back(VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLER, 10});
	pool_sizes_2.push_back(VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE , 10 });

	VkDescriptorPoolCreateInfo pool_info_2 = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
	pool_info_2.poolSizeCount = pool_sizes_2.size();
	pool_info_2.pPoolSizes = pool_sizes_2.data();
	pool_info_2.maxSets = 100;
	
	VK_CHECK(vkCreateDescriptorPool(engine->device, &pool_info_2, nullptr, &engine->descriptor_pool));
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
	CHECK(mem_idx != -1);
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
	CHECK(deviceCount != 0);
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(engine.instance, &deviceCount, devices.data());
	
	for (const auto& device : devices) {
		if (retrieveDeviceCapabilities(device, &engine.indices))
		{
			engine.physicalDevice = device;
			break;
		}
	}

	CHECK(engine.physicalDevice);


	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(engine.physicalDevice, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(engine.physicalDevice, &queueFamilyCount, queueFamilies.data());

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set <u32> indicesSet = { engine.indices.graphicsFamily, engine.indices.presentFamily };
	float queuePriority = 1.0f;
	for (u32 idx : indicesSet) {
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = idx;
		queueCreateInfo.queueCount = 1;
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
	colorAttachment.format = VK_FORMAT_B8G8R8A8_SRGB;
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
rImage& rWindowTakeScreenshot(rWindow* window)
{
	auto& engine = *window->engine;

	let image_index = window->imageIndex;
	let swapchain_image = window->swapchainImages[image_index];

	VkImageCreateInfo image_info = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	//image_info.flags; VkImageCreateFlags       flags;
	image_info.imageType = VK_IMAGE_TYPE_2D;
	image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
	image_info.extent = VkExtent3D{ window->width, window->height, 1 };

	image_info.mipLevels = 1;
	image_info.arrayLayers = 1;
	image_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_info.tiling = VK_IMAGE_TILING_LINEAR;
	image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	VkImage screenshot_image;
	
	VK_CHECK(vkCreateImage(engine.device, &image_info, nullptr, &screenshot_image));

	VkMemoryRequirements memory_requirements;
	vkGetImageMemoryRequirements(engine.device, screenshot_image, &memory_requirements);

	VkMemoryAllocateInfo memory_alloc_info = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	memory_alloc_info.allocationSize = memory_requirements.size;
	memory_alloc_info.memoryTypeIndex = rEngineGetMemoryIdx(engine, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	VkDeviceMemory memory;
	vkAllocateMemory(engine.device, &memory_alloc_info, nullptr, &memory);
	vkBindImageMemory(engine.device, screenshot_image, memory, 0);

	let command_buffer = beginSingleTimeCommands(*window->engine);
	
	
	VkImageMemoryBarrier screenshot_barrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
	screenshot_barrier.image = screenshot_image;
	screenshot_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	screenshot_barrier.subresourceRange.baseMipLevel = 0;
	screenshot_barrier.subresourceRange.levelCount = 1;
	screenshot_barrier.subresourceRange.baseArrayLayer = 0;
	screenshot_barrier.subresourceRange.layerCount = 1;

	screenshot_barrier.srcAccessMask = 0;
	screenshot_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	screenshot_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	screenshot_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

	vkCmdPipelineBarrier( command_buffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0, // dependency flags
		0, nullptr,  // memory barrier
		0, nullptr,  // buffer memory barrier
		1, &screenshot_barrier); // image barrier

	VkImageMemoryBarrier swapchain_image_barrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };

	swapchain_image_barrier.image = swapchain_image;
	swapchain_image_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	swapchain_image_barrier.subresourceRange.baseMipLevel = 0;
	swapchain_image_barrier.subresourceRange.levelCount = 1;
	swapchain_image_barrier.subresourceRange.baseArrayLayer = 0;
	swapchain_image_barrier.subresourceRange.layerCount = 1;

	swapchain_image_barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	swapchain_image_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	swapchain_image_barrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	swapchain_image_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	vkCmdPipelineBarrier( command_buffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0, // dependency flags
		0, nullptr,  // memory barrier
		0, nullptr,  // buffer memory barrier
		1, &swapchain_image_barrier); // image barrier

	VkOffset3D blit_size;
	blit_size.x = window->width;
	blit_size.y = window->height;
	blit_size.z = 1;

	let supports_blit = false; // TODO this check
	if (supports_blit)
	{
		VkImageBlit blit_region{};
		blit_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit_region.srcSubresource.baseArrayLayer = 0;
		blit_region.srcSubresource.layerCount = 1;
		blit_region.srcSubresource.mipLevel = 0;
		blit_region.srcOffsets[1] = blit_size;
		blit_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit_region.dstSubresource.baseArrayLayer = 0;
		blit_region.dstSubresource.layerCount = 1;
		blit_region.dstSubresource.mipLevel = 0;
		blit_region.dstOffsets[1] = blit_size;
		vkCmdBlitImage(command_buffer, swapchain_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, screenshot_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit_region, VK_FILTER_NEAREST);
	}
	else {
		VkImageCopy image_copy_region = {};
		image_copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		image_copy_region.srcSubresource.layerCount = 1;
		image_copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		image_copy_region.dstSubresource.layerCount = 1;
		image_copy_region.extent.width = window->width;
		image_copy_region.extent.height = window->height;
		image_copy_region.extent.depth = 1;

		vkCmdCopyImage(
			command_buffer, 
			swapchain_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 
			screenshot_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
			1, &image_copy_region);
	}


	screenshot_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	screenshot_barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	screenshot_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	screenshot_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
	vkCmdPipelineBarrier( command_buffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0, // dependency flags
		0, nullptr,  // memory barrier
		0, nullptr,  // buffer memory barrier
		1, &screenshot_barrier); // image barrier

	swapchain_image_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	swapchain_image_barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	swapchain_image_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	swapchain_image_barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	vkCmdPipelineBarrier( command_buffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0, // dependency flags
		0, nullptr,  // memory barrier
		0, nullptr,  // buffer memory barrier
		1, &swapchain_image_barrier); // image barrier

	endSingleTimeCommands(engine, command_buffer);

	VkImageSubresource subresource{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 0 };
	VkSubresourceLayout subresource_layout;
	vkGetImageSubresourceLayout(engine.device, screenshot_image, &subresource, &subresource_layout);
	
	void* data;
	vkMapMemory(engine.device, memory, 0, VK_WHOLE_SIZE, 0, &data);
	data = (char*)data + subresource_layout.offset;
	struct color { u8 r, g, b, a; };
	color* colors = (color*)data;
	if (!supports_blit) {
		for (u32 idx = 0; idx < subresource_layout.size/4; idx++)
		{
			color& color = colors[idx];
			color = { color.b, color.g, color.r, 255 }; // swap red and blue, and set alpha to 1.0
		}
	}
	
	INFO("w: %d, h: %d", window->width, window->height);

	stbi_write_png("screenshot.png", window->width, window->height, 4, data, window->width * 4);

	rImage image;
	image.image = screenshot_image;
	return image;
}

bool rEngineStartFrame(rEngine& engine)
{
	if (!rEngineShouldTick(engine)) return false;

	engine.uptime = ImGui::GetTime();
	engine.deltaTime = ImGui::GetIO().DeltaTime;

	glfwPollEvents();

	// this should be handled better for each window I guess.
	ImGui_ImplGlfw_NewFrame();

	ImGui::NewFrame();

	return true;
}


void rEngineEndFrame(rEngine& engine)
{

	auto io = ImGui::GetIO();

	if (io.KeysDown[GLFW_KEY_ESCAPE]) {
		glfwSetWindowShouldClose(engine.windows[0]->glfwWindow, true); // @hack
	}

	if (io.KeysDown[GLFW_KEY_F9] && (io.KeysDownDuration[GLFW_KEY_F9] == 0.)) {
		INFO("take screenshot");
		let image = rWindowTakeScreenshot(engine.windows[0]);
	}


	
	
	bool imgui_debug = false;
	//ImGui::ShowDemoWindow(&imgui_debug);

	ImGui::Render();
	
	vkQueueWaitIdle(engine.presentQueue);
	for (rWindow* window : engine.windows)
	{
		if (glfwWindowShouldClose(window->glfwWindow))
		{
			rDestroyWindow(window);
			continue;
		}
	}
	u32 numWindows = u32(engine.windows.size());

	std::vector<u32> indices;
	std::vector<VkSwapchainKHR> swapchains;
	std::vector<VkSemaphore> windowSemaphores;

	for (rWindow* window : engine.windows)
	{
		rWindowRender(window);
		swapchains.push_back(window->swapchain);
		indices.push_back(window->imageIndex);
		windowSemaphores.push_back(window->renderFinishedSemaphore);
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

		VK_CHECK(vkQueuePresentKHR(engine.presentQueue, &presentInfo));
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
