
#pragma once
#include "vulkan/vulkan_core.h"

#include "engine.h"
#include "types.h"


struct rBuffer {
	u32 size;
	void* data;
	VkBufferUsageFlags usage;
	VkSharingMode sharingMode;

	rEngine* engine;
	VkBuffer buffer;
	VkDeviceMemory memory;

	rBuffer() {}

	rBuffer(rEngine& inEngine, void* inData, u32 inSize, VkBufferUsageFlags inUsage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VkSharingMode inSharingMode = VK_SHARING_MODE_EXCLUSIVE);

};

template <class T>
rBuffer rBufferCreate(rEngine& engine, T& data, VkBufferUsageFlags usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE) {
	return rBuffer(engine, &data, sizeof(data), usage, sharing_mode);
}

void rBufferSync(const rBuffer& buffer);

void rBufferFetch(const rBuffer & buffer);
