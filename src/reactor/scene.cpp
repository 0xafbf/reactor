
#include "algorithm"

#include "engine.h"
#include "scene.h"
#include "geometry.h"
#include "log.h"
#include "image.h"

#include "slang.h"

void rSceneDraw(rScene* scene, VkCommandBuffer buffer) {
	for (rState* prim : scene->primitives)
	{
		rPipelineDraw(prim, buffer);
	}
}

void rStateSetDescriptor(VkDevice device, rState& state, const char* name, u32 binding, rBuffer& buffer, VkDescriptorType descriptor_type) {
	let r = state.pipeline->shader.reflection;
	u32 set_idx = -1;
	let parameter_count = spReflection_GetParameterCount(r);
	for (u32 idx = 0; idx < parameter_count; ++idx)
	{
		let param = spReflection_GetParameterByIndex(r, idx);
		let param_name = spReflectionVariableLayout_GetSemanticName(param);
		let cmp = strcmp(param_name, name);
		INFO("comparing param %s, cmp: %d", param_name, cmp);
		if (strcmp(param_name, name) == 0) {
			set_idx = idx;
		}
	}
	if (set_idx == -1) {
		return;
	}

	if (state.descriptor_sets.size() > set_idx)
	{
		rStateSetDescriptor(device, state, set_idx, binding, buffer, descriptor_type);
	}
}

// TODO: maybe in the future have some way to set descriptor by name
void rStateSetDescriptor(VkDevice device, rState& state, u32 set_idx, u32 binding, rBuffer & buffer, VkDescriptorType descriptor_type) {

	if (set_idx >= state.descriptor_sets.size()) {
		return;
	}
	auto set = state.descriptor_sets[set_idx];

	VkDescriptorBufferInfo buffer_info;
	buffer_info.buffer = buffer.buffer;
	buffer_info.offset = 0;
	buffer_info.range = 1;

	VkWriteDescriptorSet write = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
	write.dstSet = set;
	write.dstBinding = binding;
	write.dstArrayElement = 0;
	write.descriptorCount = 1;
	write.descriptorType = descriptor_type;
	//write.pImageInfo;
	write.pBufferInfo = &buffer_info;
	//write.pTexelBufferView;
	vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
}

void rStateSetDescriptor(VkDevice device, rState & state, u32 binding, rImage & image, VkDescriptorType descriptor_type) {

	VkDescriptorImageInfo descrImageInfo;
	descrImageInfo.sampler = image.sampler;
	descrImageInfo.imageView = image.imageView;
	descrImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet write = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
	write.dstSet = state.descriptor_sets[0];
	write.dstBinding = binding;
	write.dstArrayElement = 0;
	write.descriptorCount = 1;
	write.descriptorType = descriptor_type;
	write.pImageInfo = &descrImageInfo;
	//write.pBufferInfo = &buffer_info;
	//write.pTexelBufferView;
	vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
}

VkDescriptorSet rDescriptorSet(VkDevice device, VkDescriptorPool descriptor_pool, VkDescriptorSetLayout layout) {

	VkDescriptorSetAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	allocInfo.descriptorPool = descriptor_pool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &layout;

	auto descriptor_set = VkDescriptorSet();
	VK_CHECK(vkAllocateDescriptorSets(device, &allocInfo, &descriptor_set));
	return descriptor_set;
}

array<VkDescriptorSet> rDescriptorSets(VkDevice device, VkDescriptorPool descriptor_pool, array<VkDescriptorSetLayout>& layouts) {
	if (!layouts.size()) {
		return {};
	}
	VkDescriptorSetAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	allocInfo.descriptorPool = descriptor_pool;
	allocInfo.descriptorSetCount = layouts.size();
	allocInfo.pSetLayouts = layouts.data();

	auto descriptor_sets = array<VkDescriptorSet>(layouts.size());
	VK_CHECK(vkAllocateDescriptorSets(device, &allocInfo, descriptor_sets.data()));
	return descriptor_sets;
}



void rPipelineDraw(rState* state, VkCommandBuffer commandBuffer) {
	let& pipeline = state->pipeline;
	if (!pipeline->did_compile) {
		// invalid pipeline, skip this draw
		return;
	}
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipeline);
		
	VkDeviceSize offsets[] = { 0 };
	
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &state->geometry->vertexBuffer.buffer, offsets);
	vkCmdBindIndexBuffer(commandBuffer, state->geometry->indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
	if (state->descriptor_sets.size()){
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->shader.layout, 0, state->descriptor_sets.size(), state->descriptor_sets.data(), 0, nullptr);
	}
	// there is also vkCmdDraw (without indexed) that maybe is faster.
	vkCmdDrawIndexed(commandBuffer, state->geometry->indices.size(), 1, 0, 0, 0);
}

bool rDebug(VkPipelineRasterizationStateCreateInfo& rasterizer) {
	bool changed = false;
	changed |= ImGui::DragFloat("line width", &rasterizer.lineWidth, 0.1, 0, 64);
	changed |= rDebugCombo("front face", &rasterizer.frontFace, { "counter-clockwise", "clockwise" });
	changed |= rDebugCombo("cull mode", &rasterizer.cullMode, { "none", "front", "back", "front and back" });
	changed |= rDebugCombo("polygon mode", &rasterizer.polygonMode, { "fill", "line", "point" });

	return changed;
}

bool rDebug(VkPipelineInputAssemblyStateCreateInfo& input_assembly) {
	bool changed = false;
	changed |= rDebugCombo("topology", &input_assembly.topology, {
		"point list",
		"line list",
		"line strip",
		"triangle list",
		"triangle strip",
		"triangle fan",
		"line list with adjacency",
		"line strip with adjacency",
		"triangle list with adjacency",
		"triangle strip with adjacency",
		});
	return changed;
}

void rDebug(rGraphicsPipeline& graphics_pipeline) {
	bool pipeline_changed = false;
	pipeline_changed |= rDebug(graphics_pipeline.rasterizer);
	pipeline_changed |= rDebug(graphics_pipeline.input_assembly);
	if (pipeline_changed) {
		rPipelineUpdate(graphics_pipeline);
	}
}
