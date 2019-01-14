
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

#include "image.h"
#include "commandbuffer.h"


void rImageCreate(rImage& image, string location) {

	unsigned char *data = stbi_load(location.c_str(), &image.width, &image.height, &image.depth, STBI_rgb_alpha);
	assert(data);
	
	let bufferSize = image.width * image.height * 4;
	var& engine = *image.engine;

	var buffer = rBuffer(engine, data, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	rBufferSync(buffer);
	stbi_image_free(data);
	
	VkImageCreateInfo imageInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = image.width;
	imageInfo.extent.height = image.height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

	/*VkImageFormatProperties imageProps;
	VkResult r = vkGetPhysicalDeviceImageFormatProperties(engine.physicalDevice, VK_FORMAT_B8G8R8_SRGB, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_TRANSFER_DST_BIT, 0, &imageProps);

	VkFormatProperties formatProps;
	vkGetPhysicalDeviceFormatProperties(engine.physicalDevice, VK_FORMAT_R8G8B8A8_SRGB, &formatProps);*/
	VK_CHECK(vkCreateImage(engine.device, &imageInfo, nullptr, &image.image));

	VkDeviceMemory imageMemory;
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(engine.device, image.image, &memRequirements);
	VkMemoryAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = rEngineGetMemoryIdx(engine, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VK_CHECK(vkAllocateMemory(engine.device, &allocInfo, nullptr, &imageMemory));

	vkBindImageMemory(engine.device, image.image, imageMemory, 0);

	let commandBuffer = beginSingleTimeCommands(engine);

	VkImageMemoryBarrier memory_barrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
	memory_barrier.image = image.image;
	memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	memory_barrier.subresourceRange.baseMipLevel = 0;
	memory_barrier.subresourceRange.levelCount = 1;
	memory_barrier.subresourceRange.layerCount = 1;

	memory_barrier.srcAccessMask = 0;
	memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	memory_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	memory_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_HOST_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &memory_barrier);

	VkBufferImageCopy region;
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { u32(image.width), u32(image.height), 1 };

	vkCmdCopyBufferToImage(commandBuffer, buffer.buffer, image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	memory_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	memory_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &memory_barrier);

	endSingleTimeCommands(engine, commandBuffer);

	VkImageViewCreateInfo imageViewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    imageViewInfo.image = image.image;
    imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewInfo.subresourceRange.baseMipLevel = 0;
    imageViewInfo.subresourceRange.levelCount = 1;
    imageViewInfo.subresourceRange.baseArrayLayer = 0;
    imageViewInfo.subresourceRange.layerCount = 1;

	VK_CHECK(vkCreateImageView(engine.device, &imageViewInfo, nullptr, &image.imageView));

	VkSamplerCreateInfo samplerInfo = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.maxAnisotropy = 16;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.;
	samplerInfo.minLod = 0.;
	samplerInfo.maxLod = 0.;

	VK_CHECK(vkCreateSampler(engine.device, &samplerInfo, nullptr, &image.sampler));

}
