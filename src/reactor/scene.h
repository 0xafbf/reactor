#pragma once

#include "types.h"
#include "vulkan/vulkan_core.h"
#include "rmath.h"
#include "buffer.h"
#include "pipeline.h"
#include "geometry.h"

struct rState {
	rGraphicsPipeline* pipeline;
	rGeometry* geometry;
	array<VkDescriptorSet> descriptor_sets;
};


void rPipelineDraw(rState* state, VkCommandBuffer buffer);


struct rScene
{
	array<rState*> primitives;
};

void rSceneDraw(rScene* scene, VkCommandBuffer buffer);

void rStateSetDescriptor(VkDevice device, rState& state, u32 binding, rBuffer& buffer, VkDescriptorType descriptor_type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

struct rImage;
void rStateSetDescriptor(VkDevice device, rState& state, u32 binding, rImage& image, VkDescriptorType descriptor_type);

VkDescriptorSet rDescriptorSet(VkDevice device, VkDescriptorPool descriptor_pool, VkDescriptorSetLayout layout);


void rDebug(rGraphicsPipeline& graphics_pipeline);
