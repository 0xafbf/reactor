
#pragma once
#include "vulkan/vulkan.h"

#include "engine.h"
#include "types.h"
#include "scene.h"


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

	inline operator VkDescriptorBufferInfo();

};

void rBufferSync(const rBuffer& buffer);

VkDescriptorBufferInfo rDescriptorBufferInfo(rBuffer& buffer);

	inline rBuffer::operator VkDescriptorBufferInfo()
	{
		return rDescriptorBufferInfo(*this);
	}
