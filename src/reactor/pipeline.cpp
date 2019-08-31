
#include "slang.h"
#include "pipeline.h"

#include "log.h"
#include "debug.h"

static VkDevice device = 0;

VkDescriptorType rDescriptorType(SlangTypeKind kind) {
	switch (kind) {
	case SLANG_TYPE_KIND_PARAMETER_BLOCK: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	case SLANG_TYPE_KIND_MATRIX: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	case SLANG_TYPE_KIND_VECTOR: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	case SLANG_TYPE_KIND_SCALAR: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	case SLANG_TYPE_KIND_CONSTANT_BUFFER: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	case SLANG_TYPE_KIND_RESOURCE: return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	case SLANG_TYPE_KIND_SAMPLER_STATE: return VK_DESCRIPTOR_TYPE_SAMPLER;
	default:
		auto the_kind = kind;
		WARN("Unimplemented descriptor type %d", the_kind);
		break;
	}
}


// a descriptor set layout should be derived just from the shader
// type layout right?
VkDescriptorSetLayout rDescriptorSetLayout(SlangReflectionTypeLayout* type_layout) {
	auto type = spReflectionTypeLayout_GetType(type_layout);

	// maybe will be used later
	auto name = spReflectionType_GetName(type);

	auto field_count = spReflectionType_GetFieldCount(type);
	auto layout_bindings = array<VkDescriptorSetLayoutBinding>(field_count);
	for (auto idx = 0; idx < field_count; ++idx) {
		auto field = spReflectionTypeLayout_GetFieldByIndex(type_layout, idx);

		let field_type_layout = spReflectionVariableLayout_GetTypeLayout(field);
		let field_type = spReflectionTypeLayout_GetType(field_type_layout);
		let kind = spReflectionType_GetKind(field_type);
		auto descriptor_type = rDescriptorType(kind);

		// all this info can be better retrieved with reflection
		auto& layout_binding = layout_bindings[idx];
		layout_binding.binding = spReflectionParameter_GetBindingIndex(field);
		layout_binding.descriptorCount = 1;
		layout_binding.descriptorType = descriptor_type;
		layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		layout_binding.pImmutableSamplers = nullptr;
	}

	VkDescriptorSetLayoutCreateInfo layout_info = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	//layout_info.flags = 0
	layout_info.pBindings = layout_bindings.data();
	layout_info.bindingCount = layout_bindings.size(); // equals field_count

	VkDescriptorSetLayout descriptor_set_layout;
	CHECK(device);
	VK_CHECK(vkCreateDescriptorSetLayout(device, &layout_info, nullptr, &descriptor_set_layout));
	return descriptor_set_layout;
}


#include "gui.h"
static ImGuiTextBuffer  slang_log;


void diagnosticCallback(char const* message,
	void*) {
	slang_log.appendf(message);
	INFO(message);
}
static bool p_open = true;
void rShaderShowLog() {
	if (!ImGui::Begin("LOG", &p_open)) {
		ImGui::End();
		return;
	}
	ImGui::BeginChild("shader log");
	ImGui::TextUnformatted(slang_log.begin(), slang_log.end());
	ImGui::EndChild();
	ImGui::End();
}


VkShaderStageFlagBits rShaderStageFlagBits(SlangStage stage) {
	switch (stage) {
	case SLANG_STAGE_VERTEX: return VK_SHADER_STAGE_VERTEX_BIT;
	case SLANG_STAGE_FRAGMENT: return VK_SHADER_STAGE_FRAGMENT_BIT;
	default:
		CHECK(!"Unimplemented");
		return VkShaderStageFlagBits(0);
	}
};


VkFormat rFormat(size_t count, SlangScalarType type)
{
	if (type == SLANG_SCALAR_TYPE_FLOAT32) {
		switch (count) {
		case 1: return VK_FORMAT_R32_SFLOAT;
		case 2: return VK_FORMAT_R32G32_SFLOAT;
		case 3: return VK_FORMAT_R32G32B32_SFLOAT;
		case 4: return VK_FORMAT_R32G32B32A32_SFLOAT;
		}
	}
}


rShader rShaderNew(VkDevice in_device, string in_path) {
	device = in_device;

	rShader r;
	r.did_compile = false;
	r.name = in_path;

	static SlangSession* session = spCreateSession(NULL);
	SlangCompileRequest* request = spCreateCompileRequest(session);
	spSetCodeGenTarget(request, SLANG_SPIRV);
	spAddSearchPath(request, "shaders/");
	//spAddPreprocessorDefine(request, "ENABLE_FOO", "1")

	int translationUnitIndex = spAddTranslationUnit(request, SLANG_SOURCE_LANGUAGE_SLANG, nullptr); // nullptr was "" (empty string)

	spAddTranslationUnitSourceFile(request, translationUnitIndex, in_path.c_str());
	SlangProfileID profileID = spFindProfile(session, "ps_5_0");

	spSetDiagnosticCallback(request, diagnosticCallback, nullptr);

	let shader_compile_result = spCompile(request);
	INFO("shader compile result: %d", shader_compile_result);
	if (SLANG_FAILED(shader_compile_result))
	{
		// we should fallback, but don't know if here is the appropiate place
		return r;
	}

	// all from here is reflection stuff

	r.reflection = spGetReflection(request);


	int entry_point_count = spReflection_getEntryPointCount(r.reflection);

	// for (entry_points) create shader_module
	for (u32 idx = 0; idx < entry_point_count; idx++)
	{
		int entry_point_index = idx;
		size_t code_size;
		let code = spGetEntryPointCode(request, entry_point_index, &code_size);
		let entry_point = spReflection_getEntryPointByIndex(r.reflection, entry_point_index);
		// it seems the compiler always outputs entry point main
		let entry_point_name = "main";

		let slang_stage = spReflectionEntryPoint_getStage(entry_point);
		auto stage = rShaderStageFlagBits(slang_stage);

		VkShaderModuleCreateInfo shader_module_info = {};
		shader_module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shader_module_info.codeSize = code_size;
		shader_module_info.pCode = (u32*)code;

		VkShaderModule* stage_module;
		if (stage == VK_SHADER_STAGE_VERTEX_BIT) stage_module = &r.vert_module;
		else if (stage == VK_SHADER_STAGE_FRAGMENT_BIT) stage_module = &r.frag_module;
		else CHECK(false && "Unhandled shader stage");

		VK_CHECK(vkCreateShaderModule(device, &shader_module_info, nullptr, stage_module));

		VkPipelineShaderStageCreateInfo stage_info = {};
		stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stage_info.stage = stage;
		stage_info.module = *stage_module;
		stage_info.pName = entry_point_name;
		r.shader_stages.push_back(stage_info);

		u32 param_count = spReflectionEntryPoint_getParameterCount(entry_point);
		for (u32 jdx = 0; jdx < param_count; ++jdx) {
			// hack
			if (stage == VK_SHADER_STAGE_FRAGMENT_BIT) continue;
			let param = spReflectionEntryPoint_getParameterByIndex(entry_point, jdx);
			let binding_index = spReflectionParameter_GetBindingIndex(param);
			let layout = spReflectionVariableLayout_GetTypeLayout(param);
			let type = spReflectionTypeLayout_GetType(layout);
			auto stride = 0;

			let field_count = spReflectionType_GetFieldCount(type);
			for (u32 kdx = 0; kdx < field_count; ++kdx) {
				let field = spReflectionType_GetFieldByIndex(type, kdx);
				let type = spReflectionVariable_GetType(field);
				let name = spReflectionVariable_GetName(field);
				let type_name = spReflectionType_GetName(type);
				INFO("found field %s : %s", name, type_name);

				// for example, this field is:
				// is a bool/int/float
				let scalar_type = spReflectionType_GetScalarType(type);

				// it is useful if type->kind is array
				let elem_count = spReflectionType_GetElementCount(type);
				// it is a int/float 16/32/64 type!


				auto format = rFormat(elem_count, scalar_type);

				VkVertexInputAttributeDescription attribute_description;
				attribute_description.binding = binding_index;
				attribute_description.format = format;
				attribute_description.location = kdx;
				attribute_description.offset = stride;
				r.input_attributes.push_back(attribute_description);

				stride += elem_count * 4;
			}
			VkVertexInputBindingDescription binding_description = {};
			binding_description.binding = binding_index;
			binding_description.stride = stride;
			binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			r.input_bindings.push_back(binding_description);
		}
	}
	// end shader stages

	// now parse parameters
	let parameter_count = spReflection_GetParameterCount(r.reflection);
	r.descriptor_set_layouts = array<VkDescriptorSetLayout>(parameter_count);

	for (u32 idx = 0; idx < parameter_count; ++idx) {
		let parameter = spReflection_GetParameterByIndex(r.reflection, idx);

		let type_layout = spReflectionVariableLayout_GetTypeLayout(parameter);
		let elem_type_layout = spReflectionTypeLayout_GetElementTypeLayout(type_layout);
		// I think that maybe this result can be cached in the case of
		// some shaders sharing the same parameter layouts...
		auto descriptor_set_layout = rDescriptorSetLayout(elem_type_layout);

		auto parameter_index = spReflectionParameter_GetBindingSpace(parameter);
		r.descriptor_set_layouts[parameter_index] = descriptor_set_layout;
	}


	VkPipelineLayoutCreateInfo pipelineLayoutInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	pipelineLayoutInfo.setLayoutCount = r.descriptor_set_layouts.size();
	pipelineLayoutInfo.pSetLayouts = r.descriptor_set_layouts.data();
	VK_CHECK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &r.layout));
	r.did_compile = true;

	return r;
}

VkPipelineVertexInputStateCreateInfo rVertexInputInfo(const rShader& shader)
{
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
	vertexInputInfo.vertexBindingDescriptionCount = shader.input_bindings.size();
	vertexInputInfo.pVertexBindingDescriptions = shader.input_bindings.data();
	vertexInputInfo.pVertexAttributeDescriptions = shader.input_attributes.data();
	vertexInputInfo.vertexAttributeDescriptionCount = shader.input_attributes.size();
	return vertexInputInfo;
}

rGraphicsPipeline rPipeline(rEngine& inEngine, string inPath) {

	rGraphicsPipeline r;
	r.engine = &inEngine;

	r.shader = rShaderNew(inEngine.device, inPath);

	r.input_assembly = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
	r.input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	//inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
	r.input_assembly.primitiveRestartEnable = VK_FALSE;


	r.rasterizer = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
	r.rasterizer.depthClampEnable = VK_FALSE;
	r.rasterizer.rasterizerDiscardEnable = VK_FALSE;
	r.rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	r.rasterizer.lineWidth = 1.0f;
	r.rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	r.rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	r.rasterizer.depthBiasEnable = VK_FALSE;

	rPipelineUpdate(r);

	return r;
}



struct ShaderStages {
	u64 count;
	VkPipelineShaderStageCreateInfo* ptr;
};

rShader rShaderNew(VkDevice, string);




void rPipelineUpdate(rGraphicsPipeline & pipeline)
{
	auto& r = pipeline;

	if (!pipeline.shader.did_compile) {
		// shader didn't compile, nothing to do here...
		pipeline.did_compile = false;
		return;
	}

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = false;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT |
		VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlendInfo = {};
	colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendInfo.logicOpEnable = false;
	colorBlendInfo.attachmentCount = 1;
	colorBlendInfo.pAttachments = &colorBlendAttachment;

	array<VkDynamicState> dynamicStateEnables;
	dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
	dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);

	VkPipelineDynamicStateCreateInfo dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.pDynamicStates = dynamicStateEnables.data();
	dynamicState.dynamicStateCount = u32(dynamicStateEnables.size());

	VkGraphicsPipelineCreateInfo pipelineCreateInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };

	pipelineCreateInfo.stageCount = r.shader.shader_stages.size();
	pipelineCreateInfo.pStages = r.shader.shader_stages.data();

	auto vertex_input_info = rVertexInputInfo(r.shader);

	pipelineCreateInfo.pVertexInputState = &vertex_input_info;
	pipelineCreateInfo.pInputAssemblyState = &r.input_assembly;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pRasterizationState = &r.rasterizer;
	pipelineCreateInfo.pMultisampleState = &multisampling;
	pipelineCreateInfo.pColorBlendState = &colorBlendInfo;
	pipelineCreateInfo.layout = r.shader.layout;
	pipelineCreateInfo.renderPass = r.engine->primitivesRenderPass;
	pipelineCreateInfo.subpass = 0;
	pipelineCreateInfo.pDynamicState = &dynamicState;

	VK_CHECK(vkCreateGraphicsPipelines(r.engine->device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &r.pipeline));
	pipeline.did_compile = true;
}
