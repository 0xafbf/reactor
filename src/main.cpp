
#include <iostream>
#include "reactor.h"

#include "tiny_obj_loader.h"
#include <stdlib.h>

struct rBuffer {
	u32 size;
	VkBufferUsageFlags usage;
	VkSharingMode sharingMode;

	rEngine* engine;
	VkBuffer buffer;
	VkDeviceMemory memory;

	rBuffer() {}

	rBuffer(rEngine* inEngine, u32 inSize, VkBufferUsageFlags inUsage, VkSharingMode inSharingMode)
	: size(inSize), usage(inUsage), sharingMode(inSharingMode)
	, engine(inEngine)	
	{
	
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = sharingMode;
		bufferInfo.size = size;
		
		VK_CHECK(vkCreateBuffer(engine->device, &bufferInfo, nullptr, &buffer));
	
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(engine->device, buffer, &memRequirements);
		
		VkPhysicalDeviceMemoryProperties memProps;
		vkGetPhysicalDeviceMemoryProperties(engine->physicalDevice, &memProps);
		u32 mem_idx = -1;
		for (u32 idx = 0; idx < memProps.memoryTypeCount; ++idx)
		{
			VkMemoryPropertyFlags memFlags = memProps.memoryTypes[idx].propertyFlags;
			if (memFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT && memFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
			{
				mem_idx = idx;
				break;
			}
		}
		
		VkMemoryAllocateInfo memInfo = {};
		memInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memInfo.allocationSize = memRequirements.size;
		memInfo.memoryTypeIndex = mem_idx;
		
		VK_CHECK(vkAllocateMemory(engine->device, &memInfo, nullptr, &memory));
		
		VK_CHECK(vkBindBufferMemory(engine->device, buffer, memory, 0));
	}
};

void rBufferSetMemory(const rBuffer* buffer, u32 inSize, void* data)
{
	assert(inSize <= buffer->size);	
	u32 size = min(buffer->size, inSize);
	void* vkData;
	vkMapMemory(buffer->engine->device, buffer->memory, 0, size, 0, &vkData);
	memcpy(vkData, data, size);
	vkUnmapMemory(buffer->engine->device, buffer->memory);
}


struct rGeometry {

	rEngine* engine;

	tinyobj::attrib_t attrib;
	array<tinyobj::shape_t> shapes;
	array<tinyobj::material_t> materials;

	struct vert_data {
		vec3 location;
	};

	array<u32> indices;
	array<vert_data> vertices;

	rBuffer indexBuffer;
	rBuffer vertexBuffer;
	rBuffer projectionBuffer;

	rGeometry() {
	}

	rGeometry(rEngine* inEngine, string source_path):engine(inEngine) {
		string warn;
		string error;

		let ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &error, source_path.c_str());

		let& mesh = shapes[0].mesh; // @hardcoded
		let vertCount = mesh.indices.size();

		indices.resize(vertCount);
		vertices.resize(vertCount);

		for (u32 idx = 0; idx < vertCount; ++idx)
		{
			indices[idx] = idx;
			let vertIdx = mesh.indices[idx].vertex_index;
			vertices[idx].location = {
				attrib.vertices[vertIdx * 3],
				attrib.vertices[vertIdx * 3 + 1],
				attrib.vertices[vertIdx * 3 + 2],
			};
		}

		let indexBufferSize = vertCount * sizeof(u32);
		indexBuffer = rBuffer(engine, indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
		rBufferSetMemory(&indexBuffer, indexBufferSize, indices.data());

		let vertexBufferSize = vertCount * sizeof(vert_data);
		vertexBuffer = rBuffer(engine, vertexBufferSize,  VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
		rBufferSetMemory(&vertexBuffer, vertexBufferSize, vertices.data());
		
		projectionBuffer = rBuffer(engine, sizeof(mat4), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
		auto projection = mat4(1.0);
		rBufferSetMemory(&projectionBuffer, sizeof(mat4), &projection);

	}
};

void rPrimitiveFill(rPrimitive* primitive, rGeometry* geometry)
{
	primitive->indexCount = u32(geometry->indices.size());
	primitive->vertexBuffers = { geometry->vertexBuffer.buffer };
	primitive->indexBuffer = geometry->indexBuffer.buffer;
	primitive->uniformBuffers = { geometry->projectionBuffer.buffer };
}

int main()
{
	rEngine _engine("My Great App");
	rEngine* engine = &_engine;

	rWindow window(engine, "My Greatest Window", 800, 600); //name, size
	
	rGraphicsPipeline pipeline(engine, "shaders/basic.vert.spv", "shaders/basic.frag.spv");
	
	rGeometry geometry(engine, "content/monkeys.obj");
	rPrimitive primitive;
	rPrimitiveFill(&primitive, &geometry);
	primitive.pipeline = &pipeline;
	
	// create my buffers with data sorted appropiately

	
	// scene is just a container to auto draw stuff
	rScene scene;
	scene.primitives.push_back(&primitive);
	window.scene = &scene;
	
	let projectionBuffer = rBuffer(engine, sizeof(mat4), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
	auto projection = mat4(1.0);
	rBufferSetMemory(&projectionBuffer, sizeof(mat4), &projection);
	primitive.uniformBuffers = { projectionBuffer.buffer };
	
	// this sets up pipeline with data, should be moved
	VkDescriptorBufferInfo bufferInfo;
	bufferInfo.buffer = primitive.uniformBuffers[0];
	bufferInfo.offset = 0;
	bufferInfo.range = 64; // test for now
	
	VkWriteDescriptorSet descriptorWrite = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
	descriptorWrite.dstSet = pipeline.descriptorSets[0];
	descriptorWrite.dstBinding = 0;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pBufferInfo = &bufferInfo;

	vkUpdateDescriptorSets(engine->device, 1, &descriptorWrite, 0, nullptr);

	while (rEngineShouldTick(engine))
	{
		rEngineStartFrame(engine);
		
		float scroll_speed = 0.01;
		ImGui::DragFloat4("proj[0]", &projection.m[0], scroll_speed, -1.0, 1.0);
		ImGui::DragFloat4("proj[1]", &projection.m[4], scroll_speed, -1.0, 1.0);
		ImGui::DragFloat4("proj[2]", &projection.m[8], scroll_speed, -1.0, 1.0);
		ImGui::DragFloat4("proj[3]", &projection.m[12], scroll_speed, -1.0, 1.0);
	
		rBufferSetMemory(&projectionBuffer, sizeof(mat4), &projection);
		rEngineEndFrame(engine);
	}
	
	return EXIT_SUCCESS;
}

