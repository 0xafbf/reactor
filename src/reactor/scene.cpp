#include "scene.h"
#include "engine.h"



void rPrimitiveDraw(rPrimitive* primitive, rGraphicsPipeline* pipeline, VkCommandBuffer commandBuffer)
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipeline);
		
	VkDeviceSize offsets[] = { 0 };
	
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, primitive->vertexBuffers.data(), offsets);
	vkCmdBindIndexBuffer(commandBuffer, primitive->indexBuffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->layout, 0, 1, pipeline->descriptorSets.data(), 0, nullptr);
	vkCmdDraw(commandBuffer, primitive->indexCount, 1, 0, 0);
}

void rSceneDraw(rScene* scene, VkCommandBuffer buffer)
{
	for (rPrimitive* primitive : scene->primitives)
	{
		rPrimitiveDraw(primitive, primitive->pipeline, buffer);
	}
}

#include "spirv_reflect.h"
#include "algorithm"

rGraphicsPipeline::rGraphicsPipeline(rEngine* inEngine, string inVertPath, string inFragPath) :
	engine(inEngine),
	vertPath(inVertPath),
	fragPath(inFragPath)
{
	vertShader = loadFile(vertPath);
	fragShader = loadFile(fragPath);

	SpvReflectShaderModule module;
	u32 spv_size = vertShader.size();
	
	let result = spvReflectCreateShaderModule(spv_size, vertShader.data(), &module);
	assert(result == SPV_REFLECT_RESULT_SUCCESS);

	u32 var_count;
	let result2 = spvReflectEnumerateInputVariables( &module, &var_count, nullptr);
	assert(result2 == SPV_REFLECT_RESULT_SUCCESS);
	array<SpvReflectInterfaceVariable*> spv_vars(var_count);

	SPV_CHECK(spvReflectEnumerateInputVariables( &module, &var_count, spv_vars.data()));

	VkShaderModuleCreateInfo vertInfo = {};
	vertInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	vertInfo.codeSize = spvReflectGetCodeSize(&module);
	vertInfo.pCode = spvReflectGetCode(&module);
	VK_CHECK(vkCreateShaderModule(engine->device, &vertInfo, nullptr, &vertModule));

	VkShaderModuleCreateInfo fragInfo = {};
	fragInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	fragInfo.codeSize = fragShader.size();
	fragInfo.pCode = reinterpret_cast<u32*>(fragShader.data());
	VK_CHECK(vkCreateShaderModule(engine->device, &fragInfo, nullptr, &fragModule));

	VkPipelineShaderStageCreateInfo vertStageInfo = {};
	vertStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertStageInfo.module = vertModule;
	vertStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragStageInfo = {};
	fragStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragStageInfo.module = fragModule;
	fragStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertStageInfo, fragStageInfo };

	VkVertexInputBindingDescription bindingDescription = {};
	bindingDescription.binding = 0;
	bindingDescription.stride = 0; // to be filled later
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;


	u32 input_count;
	SPV_CHECK(spvReflectEnumerateEntryPointInputVariables(
			&module,
			"main",
			&input_count,
			nullptr));
	array<SpvReflectInterfaceVariable*> inputVars(input_count);
	SPV_CHECK(spvReflectEnumerateEntryPointInputVariables(
			&module,
			"main",
			&input_count,
			inputVars.data()));

	array<VkVertexInputAttributeDescription> attributeDescriptions(input_count);
	for (u32 idx = 0; idx < input_count; ++idx)
	{
		let input = inputVars[idx];
		VkVertexInputAttributeDescription& attributeDescription = attributeDescriptions[idx];
		attributeDescription.binding = bindingDescription.binding;
		attributeDescription.location = input->location;
		attributeDescription.format = VkFormat(input->format);
		attributeDescription.offset = 0; // computed after sorting
	}

	std::sort(std::begin(attributeDescriptions), std::end(attributeDescriptions),
				[](const VkVertexInputAttributeDescription& a,
				const VkVertexInputAttributeDescription& b){
				return a.location < b.location;});
	for (auto& attribute : attributeDescriptions)
	{
		u32 size = 0;

#define IMPLEMENT_CASE(fmt, fmt_size) case fmt: size = fmt_size; break;

		switch(attribute.format){
			IMPLEMENT_CASE(VK_FORMAT_UNDEFINED, 0);
			IMPLEMENT_CASE(VK_FORMAT_R32G32B32_SFLOAT, 12);
			IMPLEMENT_CASE(VK_FORMAT_R32G32B32A32_SFLOAT, 16);
		}
		assert (size != 0);
		attribute.offset = bindingDescription.stride;
		bindingDescription.stride += size;
	}
	u32 bindingCount;
	spvReflectEnumerateDescriptorBindings(&module, &bindingCount, nullptr);
	array<SpvReflectDescriptorBinding*> descriptorBindings(bindingCount);
	spvReflectEnumerateDescriptorBindings(&module, &bindingCount, descriptorBindings.data());

	array<VkDescriptorSetLayoutBinding> layoutBindings(bindingCount);
	for (u32 idx = 0; idx < bindingCount; ++idx) {
		let& binding = descriptorBindings[idx];

		VkDescriptorSetLayoutBinding& layoutBinding = layoutBindings[idx];
		layoutBinding.binding = binding->binding;
		layoutBinding.descriptorType = VkDescriptorType(binding->descriptor_type);
		layoutBinding.descriptorCount = binding->count;

		layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		layoutBinding.pImmutableSamplers = nullptr;
	}

	VkDescriptorSetLayoutCreateInfo layoutInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
	layoutInfo.bindingCount = layoutBindings.size();
	layoutInfo.pBindings = layoutBindings.data();
	VK_CHECK(vkCreateDescriptorSetLayout(engine->device, &layoutInfo, nullptr, &descriptorSetLayout));

	VkDescriptorPoolSize poolSize;
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = 1;

	VkDescriptorPoolCreateInfo poolInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = 1;
	
	VkDescriptorPool descriptorPool;

	VK_CHECK(vkCreateDescriptorPool(engine->device, &poolInfo, nullptr, &descriptorPool));

	VkDescriptorSetAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &descriptorSetLayout;
	descriptorSets.resize(1);
	VK_CHECK(vkAllocateDescriptorSets(engine->device, &allocInfo, descriptorSets.data()));

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
	vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_LINE;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
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

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

	VK_CHECK(vkCreatePipelineLayout(engine->device, &pipelineLayoutInfo, nullptr, &layout));

	array<VkDynamicState> dynamicStateEnables;
	dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
	dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);

	VkPipelineDynamicStateCreateInfo dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.pDynamicStates = dynamicStateEnables.data();
	dynamicState.dynamicStateCount = u32(dynamicStateEnables.size());

	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
	pipelineCreateInfo.stageCount = 2;
	pipelineCreateInfo.pStages = shaderStages;
	pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
	pipelineCreateInfo.pInputAssemblyState = &inputAssembly;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pRasterizationState = &rasterizer;
	pipelineCreateInfo.pMultisampleState = &multisampling;
	pipelineCreateInfo.pColorBlendState = &colorBlendInfo;
	pipelineCreateInfo.layout = layout;
	pipelineCreateInfo.renderPass = engine->primitivesRenderPass;
	pipelineCreateInfo.subpass = 0;
	pipelineCreateInfo.pDynamicState = &dynamicState;

	VK_CHECK(vkCreateGraphicsPipelines(engine->device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline));
}

rGraphicsPipeline::~rGraphicsPipeline()
{
	vkDestroyShaderModule(engine->device, vertModule, nullptr);
	vkDestroyShaderModule(engine->device, fragModule, nullptr);
}
