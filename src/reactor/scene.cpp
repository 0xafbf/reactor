
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

const char* rStageName(SlangStage stage) {
	switch (stage) {
	case SLANG_STAGE_NONE:
		return "Stage:NONE";
	case SLANG_STAGE_VERTEX:
		return "Stage:VERTEX";
	case SLANG_STAGE_HULL:
		return "Stage:HULL";
	case SLANG_STAGE_DOMAIN:
		return "Stage:DOMAIN";
	case SLANG_STAGE_GEOMETRY:
		return "Stage:GEOMETRY";
	case SLANG_STAGE_FRAGMENT:
		return "Stage:FRAGMENT";
	case SLANG_STAGE_COMPUTE:
		return "Stage:COMPUTE";
	case SLANG_STAGE_RAY_GENERATION:
		return "Stage:RAY_GENERATION";
	case SLANG_STAGE_INTERSECTION:
		return "Stage:INTERSECTION";
	case SLANG_STAGE_ANY_HIT:
		return "Stage:ANY_HIT";
	case SLANG_STAGE_CLOSEST_HIT:
		return "Stage:CLOSEST_HIT";
	case SLANG_STAGE_MISS:
		return "Stage:MISS";
	case SLANG_STAGE_CALLABLE:
		return "Stage:CALLABLE";
	}
	return "Stage:NOTFOUND";
}



const char* rScalarTypeName(SlangScalarType scalar_type){
	switch (scalar_type){
		case SLANG_SCALAR_TYPE_NONE: return "NONE";
		case SLANG_SCALAR_TYPE_VOID: return "VOID";
		case SLANG_SCALAR_TYPE_BOOL: return "BOOL";
		case SLANG_SCALAR_TYPE_INT32: return "INT32";
		case SLANG_SCALAR_TYPE_UINT32: return "UINT32";
		case SLANG_SCALAR_TYPE_INT64: return "INT64";
		case SLANG_SCALAR_TYPE_UINT64: return "UINT64";
		case SLANG_SCALAR_TYPE_FLOAT16: return "FLOAT16";
		case SLANG_SCALAR_TYPE_FLOAT32: return "FLOAT32";
		case SLANG_SCALAR_TYPE_FLOAT64: return "FLOAT64";
		case SLANG_SCALAR_TYPE_INT8: return "INT8";
		case SLANG_SCALAR_TYPE_UINT8: return "UINT8";
		case SLANG_SCALAR_TYPE_INT16: return "INT16";
		case SLANG_SCALAR_TYPE_UINT16: return "UINT16";
	}
	return "ERROR";
}


void rDebug(SlangReflectionVariable* variable){

	let variable_name = spReflectionVariable_GetName(variable);
	ImGui::Text("Field: %s", variable_name);
	// ImGui::Text("Var %s : %s(%d)", variable_name, parameter_semantic_name, parameter_semantic_index);

	let type = spReflectionVariable_GetType(variable);
	let type_name = spReflectionType_GetName(type);
	let type_kind = spReflectionType_GetKind(type);
	let type_scalar_type = spReflectionType_GetScalarType(type);
	let type_element_type = spReflectionType_GetElementType(type);
	let type_element_type_name = spReflectionType_GetName(type_element_type);

	// let type_layout = spReflectionVariableLayout_GetTypeLayout(parameter);
	let type_field_count = spReflectionType_GetFieldCount(type);

	ImGui::SameLine(200.0);
	ImGui::Text("Type (%d) %s : %s", type_field_count, type_name, type_element_type_name);
	ImGui::Indent();
	for (u32 kdx = 0; kdx < type_field_count; ++kdx) {
		let inner_variable = spReflectionType_GetFieldByIndex(type, kdx);

		rDebug(inner_variable);
		// let inner_variable = spReflectionVariableLayout_GetVariable(inner_variable_layout);

	}
	ImGui::Unindent();

}


void rDebug(rGraphicsPipeline& graphics_pipeline) {
	ImGui::Begin("pipeline");
	bool pipeline_changed = false;
	pipeline_changed |= rDebug(graphics_pipeline.rasterizer);
	pipeline_changed |= rDebug(graphics_pipeline.input_assembly);

	let reflection = graphics_pipeline.shader.reflection;
	ImGui::Text(graphics_pipeline.shader.name.c_str());

	let entry_point_count = spReflection_getEntryPointCount(reflection);
	ImGui::Text("Entry points: %d", entry_point_count);
	ImGui::Indent();
	for (u32 idx = 0; idx < entry_point_count; ++idx) {
		let entry_point = spReflection_getEntryPointByIndex(reflection, idx);
		let entry_point_name = spReflectionEntryPoint_getName(entry_point);
		let entry_point_stage = spReflectionEntryPoint_getStage(entry_point);
		let stage_name = rStageName(entry_point_stage);

		ImGui::Text("%s %s", stage_name, entry_point_name);

		let parameter_count = spReflectionEntryPoint_getParameterCount(entry_point);
		for (u32 jdx = 0; jdx < parameter_count; ++jdx) {
			let parameter = spReflectionEntryPoint_getParameterByIndex(entry_point, jdx);
			//let parameter_offset = spReflectionVariableLayout_GetOffset(parameter,);
			let parameter_semantic_name = spReflectionVariableLayout_GetSemanticName(parameter);
			let parameter_semantic_index = spReflectionVariableLayout_GetSemanticIndex(parameter);
			let variable = spReflectionVariableLayout_GetVariable(parameter);
			rDebug(variable);

		}

	}
	ImGui::Unindent();

	ImGui::Text("Shader Parameters:");

	ImGui::Indent();

	u32 param_count = spReflection_GetParameterCount(reflection);
	for (u32 idx = 0; idx < param_count; ++idx) {
		auto parameter = spReflection_GetParameterByIndex(reflection, idx);
		u32 index = spReflectionParameter_GetBindingIndex(parameter);
		u32 space = spReflectionParameter_GetBindingSpace(parameter);
		let variable = spReflectionVariableLayout_GetVariable(parameter);
		auto variable_name = spReflectionVariable_GetName(variable);

		let type_layout = spReflectionVariableLayout_GetTypeLayout(parameter);
		let type = spReflectionTypeLayout_GetType(type_layout);
		let category_count = spReflectionTypeLayout_GetCategoryCount(type_layout);
		let elem_type_layout = spReflectionTypeLayout_GetElementTypeLayout(type_layout);
		let elem_type = spReflectionTypeLayout_GetType(elem_type_layout);
		let elem_type_name = spReflectionType_GetName(elem_type);
		ImGui::Text("(%d) %s : %s", space, variable_name, elem_type_name);

		ImGui::Indent();
		let elem_field_count = spReflectionType_GetFieldCount(elem_type);
		for (u32 jdx = 0; jdx < elem_field_count; ++jdx) {
			let elem_field = spReflectionType_GetFieldByIndex(elem_type, jdx);
			rDebug(elem_field);
		}
		ImGui::Unindent();

	}
	ImGui::Unindent();
	if (pipeline_changed) {
		rPipelineUpdate(graphics_pipeline);
	}
	ImGui::End();
}
