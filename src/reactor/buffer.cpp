
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

VkDescriptorBufferInfo rDescriptorBufferInfo(rBuffer & buffer) {
	VkDescriptorBufferInfo bufferInfo;
	bufferInfo.buffer = buffer.buffer;
	bufferInfo.offset = 0;
	bufferInfo.range = buffer.size;
	return bufferInfo;
}

VkWriteDescriptorSet rDescriptorWrite(rGraphicsPipeline& pipeline, rWriteDescriptorSet& data) {
	VkWriteDescriptorSet r = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
	r.dstSet = pipeline.descriptorSets[data.setIndex];
	r.dstBinding = data.binding;
	if (data.bUseImage) {
		r.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		r.descriptorCount = data.imageInfo.size();
		r.pImageInfo = data.imageInfo.data();
	}
	else {
		r.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		r.descriptorCount = data.bufferInfo.size();
		r.pBufferInfo = data.bufferInfo.data();
	}
	return r;
}

void rPipelineUpdateDescriptorSets(rGraphicsPipeline & pipeline, array<rWriteDescriptorSet> rWrites)
{
	array<VkWriteDescriptorSet> descriptorWrites(rWrites.size());
	for (u32 idx = 0; idx < rWrites.size(); ++idx) {
		descriptorWrites[idx] = rDescriptorWrite(pipeline, rWrites[idx]);
	}
	vkUpdateDescriptorSets(pipeline.engine->device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
}

