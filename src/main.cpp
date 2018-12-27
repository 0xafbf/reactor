
#include "reactor.h"
#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )

void rStateSetDescriptor(VkDevice device, rState& state, u32 binding, rBuffer& buffer, VkDescriptorType descriptor_type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {

	VkDescriptorBufferInfo buffer_info;
	buffer_info.buffer = buffer.buffer;
	buffer_info.offset = 0;
	buffer_info.range = 1;

	VkWriteDescriptorSet write = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
	write.dstSet = state.descriptor_sets[0];
    write.dstBinding = binding;
    write.dstArrayElement = 0;
    write.descriptorCount = 1;
    write.descriptorType = descriptor_type;
    //write.pImageInfo;
	write.pBufferInfo = &buffer_info;
    //write.pTexelBufferView;
	vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
}

void rStateSetDescriptor(VkDevice device, rState& state, u32 binding, rImage& image) {

	VkDescriptorImageInfo descrImageInfo;
	descrImageInfo.sampler = image.sampler;
	descrImageInfo.imageView = image.imageView;
	descrImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet write = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
	write.dstSet = state.descriptor_sets[0];
	write.dstBinding = binding;
	write.dstArrayElement = 0;
	write.descriptorCount = 1;
	write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	write.pImageInfo = &descrImageInfo;
	//write.pBufferInfo = &buffer_info;
	//write.pTexelBufferView;
	vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
}

int main()
{
	var engine = rEngine("My Great App");
	var window = rWindow(engine, "My Greatest Window", 1024, 1024); 
	var pipeline = rPipeline(engine, "shaders/basic.slang");
	
	var pool_sizes = array<VkDescriptorPoolSize>();
	pool_sizes.push_back(VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10});
	pool_sizes.push_back(VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLER, 10});
	pool_sizes.push_back(VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE , 10 });

	VkDescriptorPoolCreateInfo poolInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
	poolInfo.poolSizeCount = pool_sizes.size();
	poolInfo.pPoolSizes = pool_sizes.data();
	poolInfo.maxSets = 100;
	
	VkDescriptorPool descriptorPool;
	VK_CHECK(vkCreateDescriptorPool(engine.device, &poolInfo, nullptr, &descriptorPool));

	VkDescriptorSetAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = pipeline.descriptor_set_layouts.size();
	allocInfo.pSetLayouts = pipeline.descriptor_set_layouts.data();

	var descriptor_sets = array<VkDescriptorSet>(pipeline.descriptor_set_layouts.size());
	VK_CHECK(vkAllocateDescriptorSets(engine.device, &allocInfo, descriptor_sets.data()));

	
	var geometry = rGeometry(engine, "content/cube.obj");

	rState state;
	state.pipeline = &pipeline;
	state.geometry = &geometry;
	state.descriptor_sets = descriptor_sets;

	rScene scene;
	scene.primitives.push_back(&state);
	window.scene = &scene;
	
	// per instance
	var transform = rTransform();
	var transform_mat = rTransformMatrix(transform);
	var transform_buffer = rBuffer(engine, &transform_mat, sizeof(mat4));
	rStateSetDescriptor(engine.device, state, 0, transform_buffer);

	var image = rImage(engine, "content/uv.png");
	rStateSetDescriptor(engine.device, state, 2, image);

	// per scene
	var camera = rOrbitCamera();
	var projection = mat4(0.);
	var projection_buffer = rBuffer(engine, &projection, sizeof(mat4));
	rStateSetDescriptor(engine.device, state, 1, projection_buffer);


	while ( rEngineStartFrame(engine))
	{
		rDebug(transform, "transform");
		transform_mat = rTransformMatrix(transform);
		rBufferSync(transform_buffer);

		rCameraTick(camera);
		float aspect_ratio = float(window.width) / float(window.height);
		projection = rCameraProject(camera, aspect_ratio) * mat4::screen();
		rBufferSync(projection_buffer);

		rEngineEndFrame(engine);
	}
	
	return 0;
}

