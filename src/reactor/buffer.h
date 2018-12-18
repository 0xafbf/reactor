
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

template<class T>
struct rSlice
{
	T* data;
	u32 size;

	rSlice(const array<T> inArray)
		: data(inArray.data())
		, size(inArray.size())
	{}

};
typedef rSlice<void> slice;

struct rGraphicsPipeline;
struct rWriteDescriptorSet {
	u32 setIndex;
	u32 binding;
	bool bUseImage;
	array<VkDescriptorBufferInfo> bufferInfo;
	array<VkDescriptorImageInfo> imageInfo;

	rWriteDescriptorSet() {}

	rWriteDescriptorSet(u32 inSetIndex, u32 inBinding, VkDescriptorBufferInfo inBufferInfo)
	{
		setIndex = inSetIndex;
		binding = inBinding;
		bufferInfo = { inBufferInfo };
		bUseImage = false;
		
	}

	rWriteDescriptorSet(u32 inSetIndex, u32 inBinding, VkDescriptorImageInfo inImageInfo)
	{
		setIndex = inSetIndex;
		binding = inBinding;
		imageInfo = { inImageInfo };
		bUseImage = true;
	}
};

void rPipelineUpdateDescriptorSets(rGraphicsPipeline & pipeline, array<rWriteDescriptorSet> rWrites);
