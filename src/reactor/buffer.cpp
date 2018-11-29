
#include "buffer.h"
#include "rmath.h"

rBuffer::rBuffer(rEngine* inEngine, u32 inSize, VkBufferUsageFlags inUsage, VkSharingMode inSharingMode)
	: size(inSize), usage(inUsage), sharingMode(inSharingMode)
	, engine(inEngine)	{

	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = sharingMode;
	bufferInfo.size = size;
	
	VK_CHECK(vkCreateBuffer(engine->device, &bufferInfo, nullptr, &buffer));

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(engine->device, buffer, &memRequirements);
	
	VkPhysicalDeviceMemoryProperties memProps;
	vkGetPhysicalDeviceMemoryProperties(engine->physicalDevice, &memProps);
	u32 mem_idx = -1;
	for (u32 idx = 0; idx < memProps.memoryTypeCount; ++idx)
	{
		VkMemoryPropertyFlags memFlags = memProps.memoryTypes[idx].propertyFlags;
		if (memFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT && memFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
		{
			mem_idx = idx;
			break;
		}
	}
	
	VkMemoryAllocateInfo memInfo = {};
	memInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memInfo.allocationSize = memRequirements.size;
	memInfo.memoryTypeIndex = mem_idx;
	
	VK_CHECK(vkAllocateMemory(engine->device, &memInfo, nullptr, &memory));
	
	VK_CHECK(vkBindBufferMemory(engine->device, buffer, memory, 0));
}

void rBufferSetMemory(const rBuffer* buffer, u32 inSize, void* data)
{
	assert(inSize <= buffer->size);	
	u32 size = min(buffer->size, inSize);
	void* vkData;
	vkMapMemory(buffer->engine->device, buffer->memory, 0, size, 0, &vkData);
	memcpy(vkData, data, size);
	vkUnmapMemory(buffer->engine->device, buffer->memory);
}
