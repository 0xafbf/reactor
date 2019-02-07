
#include "slang.h"
#include "pipeline.h"

#include "log.h"
#include "debug.h"


array<VkDescriptorSetLayoutBinding> rDescriptorLayoutBindings(rShader& shader) {

	let parameter_count = spReflection_GetParameterCount(shader.reflection);  // these are all the uniform bindings I guess
	array<VkDescriptorSetLayoutBinding> layout_bindings;

	for (u32 idx = 0; idx < parameter_count; ++idx) {
		let parameter = spReflection_GetParameterByIndex(shader.reflection, idx);
		let binding = spReflectionParameter_GetBindingIndex(parameter);

		let variable = spReflectionVariableLayout_GetVariable(parameter);
		let variable_name = spReflectionVariable_GetName(variable);

		let layout = spReflectionVariableLayout_GetTypeLayout(parameter);
		let type = spReflectionTypeLayout_GetType(layout);
		let type_name = spReflectionType_GetName(type);

		INFO("layouts variable %s : %s index %d", variable_name, type_name, binding);

		let category_count = spReflectionTypeLayout_GetCategoryCount(layout); // seems always 1
		let elem_type_layout = spReflectionTypeLayout_GetElementTypeLayout(layout);
		let elem_var_layout = spReflectionTypeLayout_GetElementVarLayout(layout);

		let category = spReflectionTypeLayout_GetParameterCategory(layout);
		if (category == SLANG_PARAMETER_CATEGORY_UNIFORM) continue;// INFO("uniform");
		if (category == SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT) INFO("descriptor table slot");
		let size = spReflectionTypeLayout_GetSize(layout, category);

		let kind = spReflectionType_GetKind(type);

		auto descriptor_type = VkDescriptorType(0);
		if (kind == SLANG_TYPE_KIND_CONSTANT_BUFFER) descriptor_type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		if (kind == SLANG_TYPE_KIND_RESOURCE) descriptor_type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		if (kind == SLANG_TYPE_KIND_SAMPLER_STATE) descriptor_type = VK_DESCRIPTOR_TYPE_SAMPLER;

		VkDescriptorSetLayoutBinding layout_binding;
		layout_binding.binding = binding;
		layout_binding.descriptorCount = 1;
		layout_binding.descriptorType = descriptor_type;
		layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		layout_binding.pImmutableSamplers = nullptr;
		layout_bindings.push_back(layout_binding);
	}
	return layout_bindings;
}



rShader rShaderNew(VkDevice device, string in_path) {

	rShader r;

	SlangSession* session = spCreateSession(NULL);
	SlangCompileRequest* request = spCreateCompileRequest(session);
	spSetCodeGenTarget(request, SLANG_SPIRV);
	spAddSearchPath(request, "shaders/");
	//spAddPreprocessorDefine(request, "ENABLE_FOO", "1")

	int translationUnitIndex = spAddTranslationUnit(request, SLANG_SOURCE_LANGUAGE_SLANG, nullptr); // nullptr was "" (empty string)

	spAddTranslationUnitSourceFile(request, translationUnitIndex, in_path.c_str());
	SlangProfileID profileID = spFindProfile(session, "ps_5_0");

	int anyErrors = spCompile(request);
	CHECK(anyErrors >= 0);
	char const* diagnostics = spGetDiagnosticOutput(request);

	INFO("Slang diagnostic err: %d, diagnostic: %s", anyErrors, diagnostics);


	r.reflection = spGetReflection(request);
	int entry_point_count = spReflection_getEntryPointCount(r.reflection);
	for (u32 idx = 0; idx < entry_point_count; idx++)
	{
		int entry_point_index = idx;
		size_t code_size;
		let code = spGetEntryPointCode(request, entry_point_index, &code_size);
		let entry_point = spReflection_getEntryPointByIndex(r.reflection, entry_point_index);
		let entry_point_name = "main";  // it seems the compiler always outputs entry point main

		let slang_stage = spReflectionEntryPoint_getStage(entry_point);
		auto stage = VkShaderStageFlagBits(0);

		switch (slang_stage) {
		case SLANG_STAGE_VERTEX: stage = VK_SHADER_STAGE_VERTEX_BIT; break;
		case SLANG_STAGE_FRAGMENT: stage = VK_SHADER_STAGE_FRAGMENT_BIT; break;
		}

		VkShaderModuleCreateInfo shader_module_info = {};
		shader_module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shader_module_info.codeSize = code_size;
		shader_module_info.pCode = (u32*)code;

		VkShaderModule* stage_module;
		if (stage == VK_SHADER_STAGE_VERTEX_BIT) stage_module = &r.vert_module;
		if (stage == VK_SHADER_STAGE_FRAGMENT_BIT) stage_module = &r.frag_module;
		CHECK(stage_module);
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
			let category = spReflectionParameter_GetBindingIndex(param);
			let type = spReflectionTypeLayout_GetType(layout);
			auto stride = 0;

			let field_count = spReflectionType_GetFieldCount(type);
			for (u32 kdx = 0; kdx < field_count; ++kdx) {
				let field = spReflectionType_GetFieldByIndex(type, kdx);
				let field_type = spReflectionVariable_GetType(field);
				INFO("found field %s : %s", spReflectionVariable_GetName(field), spReflectionType_GetName(field_type));

				let scalar_type = spReflectionType_GetScalarType(field_type);
				let elem_count = spReflectionType_GetElementCount(field_type);
				let elem_type = spReflectionType_GetElementType(field_type);

				let elem_type_count = spReflectionType_GetElementCount(elem_type);
				let elem_arst = spReflectionType_GetScalarType(elem_type);

				auto format = VK_FORMAT_R32_SFLOAT;
				if (elem_count == 2) format = VK_FORMAT_R32G32_SFLOAT;
				if (elem_count == 3) format = VK_FORMAT_R32G32B32_SFLOAT;
				if (elem_count == 4) format = VK_FORMAT_R32G32B32A32_SFLOAT;

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

	auto layout_bindings = rDescriptorLayoutBindings(r);


	VkDescriptorSetLayoutCreateInfo layout_info = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	//layout_info.flags = 0
	layout_info.pBindings = layout_bindings.data();
	layout_info.bindingCount = layout_bindings.size();

	VkDescriptorSetLayout descriptor_set_layout;
	VK_CHECK(vkCreateDescriptorSetLayout(device, &layout_info, nullptr, &descriptor_set_layout));
	r.descriptor_set_layouts.push_back(descriptor_set_layout);

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	pipelineLayoutInfo.setLayoutCount = r.descriptor_set_layouts.size();
	pipelineLayoutInfo.pSetLayouts = r.descriptor_set_layouts.data();
	VK_CHECK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &r.layout));

	return r;
}

VkPipelineVertexInputStateCreateInfo rVertexInputInfo(rShader& shader)
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
	

	
	auto& inputAssembly = r.input_assembly;
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	//inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	auto& rasterizer = r.rasterizer;
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

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

	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
	
	pipelineCreateInfo.stageCount = r.shader.shader_stages.size();
	pipelineCreateInfo.pStages = r.shader.shader_stages.data();
	auto vertex_input_info = rVertexInputInfo(r.shader);
	pipelineCreateInfo.pVertexInputState = &vertex_input_info;
	pipelineCreateInfo.pInputAssemblyState = &inputAssembly;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pRasterizationState = &rasterizer;
	pipelineCreateInfo.pMultisampleState = &multisampling;
	pipelineCreateInfo.pColorBlendState = &colorBlendInfo;
	pipelineCreateInfo.layout = r.shader.layout;
	pipelineCreateInfo.renderPass = r.engine->primitivesRenderPass;
	pipelineCreateInfo.subpass = 0;
	pipelineCreateInfo.pDynamicState = &dynamicState;

	VK_CHECK(vkCreateGraphicsPipelines(r.engine->device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &r.pipeline));

	return r;
}

void rPipelineUpdate(rGraphicsPipeline & pipeline)
{
	auto& r = pipeline;

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	//inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

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
	pipelineCreateInfo.pInputAssemblyState = &inputAssembly;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pRasterizationState = &r.rasterizer;
	pipelineCreateInfo.pMultisampleState = &multisampling;
	pipelineCreateInfo.pColorBlendState = &colorBlendInfo;
	pipelineCreateInfo.layout = r.shader.layout;
	pipelineCreateInfo.renderPass = r.engine->primitivesRenderPass;
	pipelineCreateInfo.subpass = 0;
	pipelineCreateInfo.pDynamicState = &dynamicState;

	VK_CHECK(vkCreateGraphicsPipelines(r.engine->device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &r.pipeline));


}
