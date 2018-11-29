
#pragma once
#include "vulkan/vulkan.h"

#include "engine.h"
#include "types.h"


struct rBuffer {
	u32 size;
	VkBufferUsageFlags usage;
	VkSharingMode sharingMode;

	rEngine* engine;
	VkBuffer buffer;
	VkDeviceMemory memory;

	rBuffer() {}

	rBuffer(rEngine* inEngine, u32 inSize, VkBufferUsageFlags inUsage, VkSharingMode inSharingMode);
};

void rBufferSetMemory(const rBuffer* buffer, u32 inSize, void* data);
