
#include "buffer.h"
#include "rmath.h"

rBuffer::rBuffer(rEngine& inEngine, void* inData, u32 inSize, VkBufferUsageFlags inUsage, VkSharingMode inSharingMode)
	: engine(&inEngine)
	, data(inData), size(inSize), usage(inUsage), sharingMode(inSharingMode)
	{

	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = sharingMode;
	bufferInfo.size = size;
	
	VK_CHECK(vkCreateBuffer(engine->device, &bufferInfo, nullptr, &buffer));

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(engine->device, buffer, &memRequirements);
	
	u32 mem_idx = rEngineGetMemoryIdx(*engine, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	VkMemoryAllocateInfo memInfo = {};
	memInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memInfo.allocationSize = memRequirements.size;
	memInfo.memoryTypeIndex = mem_idx;
	
	VK_CHECK(vkAllocateMemory(engine->device, &memInfo, nullptr, &memory));
	
	VK_CHECK(vkBindBufferMemory(engine->device, buffer, memory, 0));

}

void rBufferSync(const rBuffer& buffer)
{
	void* vkData;
	vkMapMemory(buffer.engine->device, buffer.memory, 0, buffer.size, 0, &vkData);
	memcpy(vkData, buffer.data, buffer.size);
	vkUnmapMemory(buffer.engine->device, buffer.memory);
}

void rBufferFetch(const rBuffer& buffer)
{
	void* vk_data;
	vkMapMemory(buffer.engine->device, buffer.memory, 0, buffer.size, 0, &vk_data);
	memcpy(buffer.data, vk_data, buffer.size);
	vkUnmapMemory(buffer.engine->device, buffer.memory);
}

VkDescriptorBufferInfo rDescriptorBufferInfo(rBuffer & buffer) {
	VkDescriptorBufferInfo bufferInfo;
	bufferInfo.buffer = buffer.buffer;
	bufferInfo.offset = 0;
	bufferInfo.range = buffer.size;
	return bufferInfo;
}
