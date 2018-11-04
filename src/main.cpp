
#include <iostream>
#include "reactor.h"


int main()
{
	rEngine _engine("My Great App");
	rEngine* engine = &_engine;

	rWindow window(engine, "My Greatest Window", 800, 600); //name, size
	
	rPrimitive prim(engine, "../shaders/vert.spv", "../shaders/frag.spv");
	rScene scene;
	scene.primitives.push_back(&prim);
	window.scene = &scene;
	
	vec3 color1(1,0,0);
	vec3 color2(0,1,0);
	vec3 color3(0,0,1);
	vec3 p1(-0.5, 0.5, 0);
	vec3 p2( 0.0, -0.5, 0);
	vec3 p3( 0.5, 0.5, 0);
	
	array<vert_data> data = {
		{p1, color1},
		{p2, color2},
		{p3, color3},
	};
	
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferInfo.size = sizeof(vert_data) * data.size();
	
	VkBuffer buffer;
	VK_CHECK(vkCreateBuffer(engine->device, &bufferInfo, nullptr, &buffer));
	
	prim.vertexBuffers.push_back(buffer);
	
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
	
	VkDeviceMemory memory;
	VK_CHECK(vkAllocateMemory(engine->device, &memInfo, nullptr, &memory));
	
	VK_CHECK(vkBindBufferMemory(engine->device, buffer, memory, 0));
	

	
	
	while (rEngineShouldTick(engine))
	{
		rEngineStartFrame(engine);
		
		
		ImGui::SliderFloat3("p1", &p1.x, -1, 1);
		ImGui::ColorPicker3("color1", &color1.x, ImGuiColorEditFlags_Float);
		ImGui::SliderFloat3("p2", &p2.x, -1, 1);
		ImGui::ColorPicker3("color2", &color2.x, ImGuiColorEditFlags_Float);
		ImGui::SliderFloat3("p3", &p3.x, -1, 1);
		ImGui::ColorPicker3("color3", &color3.x, ImGuiColorEditFlags_Float);
		array<vert_data> data = {
			{p1, color1},
			{p2, color2},
			{p3, color3},
		};
	
		void* vkData;
		vkMapMemory(engine->device, memory, 0, bufferInfo.size, 0, &vkData);
		memcpy(vkData, data.data(), bufferInfo.size);
		vkUnmapMemory(engine->device, memory);
		
		
		ImGui::BeginMainMenuBar();
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New", "a"))
			{
				std::cout << "holaa";
			}
			ImGui::Button("Load");
			ImGui::ColorPicker3("color3", &color3.x, ImGuiColorEditFlags_Float);
			ImGui::EndMenu();
			
		}
		
		ImGui::EndMainMenuBar();
		
		rEngineEndFrame(engine);
	}
	
	
	return EXIT_SUCCESS;
}

