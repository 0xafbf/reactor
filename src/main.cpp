
#include <iostream>
#include "reactor.h"

#include <stdlib.h>


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
	var model = mat4(1.0);
	var view = mat4(1.0);
	var projection = mat4(1.0);
	projection(0, 3) = 1.0f;
	var screen = mat4(0.0);
	screen(0, 2) = 1.0f;
	screen(1, 0) = 1.0f;
	screen(2, 1) = -1.0f;
	screen(3, 3) = 1.0f;



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


	var modelLocation = vec3(0);
	var viewLocation = vec3(-5.0, 0.0, 0.0);
	var viewForward = vec3(1.0, 0.0, 0.0);
	var viewUp = vec3(0.0, 0.0f, 1.0);


	while (rEngineShouldTick(engine))
	{
		rEngineStartFrame(engine);
		
		float scroll_speed = 0.01;
		ImGui::DragFloat3("location", &modelLocation.x, scroll_speed);

		model = mat4::location(modelLocation);

		ImGui::DragFloat4("model[0]", &model.m[0], scroll_speed, -1.0, 1.0);
		ImGui::DragFloat4("model[1]", &model.m[4], scroll_speed, -1.0, 1.0);
		ImGui::DragFloat4("model[2]", &model.m[8], scroll_speed, -1.0, 1.0);
		ImGui::DragFloat4("model[3]", &model.m[12], scroll_speed, -1.0, 1.0);
	
		ImGui::DragFloat3("viewLocation", &viewLocation.x, scroll_speed);

/*
		ImGui::DragFloat3("viewFwd", &viewForward.x, scroll_speed);
		ImGui::DragFloat3("viewUp", &viewUp.x, scroll_speed);
*/
		viewForward = vec3(-viewLocation.x, -viewLocation.y, -viewLocation.z);
		viewForward = viewForward.normalized();
		//viewUp = viewUp.normalized();
		viewUp = vec3(0.0, 0.0, 1.0);
		let viewRight = vec3::cross(viewUp, viewForward);
		viewUp = vec3::cross(viewForward, viewRight);
		
		//view = mat4::location(viewLocation );
		//////
		view(0, 0) = viewForward.x;
		view(0, 1) = viewForward.y;
		view(0, 2) = viewForward.z;

		view(1, 0) = viewRight.x;
		view(1, 1) = viewRight.y;
		view(1, 2) = viewRight.z;

		view(2, 0) = viewUp.x;
		view(2, 1) = viewUp.y;
		view(2, 2) = viewUp.z;

		////


		ImGui::DragFloat4("view[0]", &view.m[0], scroll_speed, -1.0, 1.0);
		ImGui::DragFloat4("view[1]", &view.m[4], scroll_speed, -1.0, 1.0);
		ImGui::DragFloat4("view[2]", &view.m[8], scroll_speed, -1.0, 1.0);
		ImGui::DragFloat4("view[3]", &view.m[12], scroll_speed, -1.0, 1.0);
	
		ImGui::DragFloat4("proj[0]", &projection.m[0], scroll_speed, -1.0, 1.0);
		ImGui::DragFloat4("proj[1]", &projection.m[4], scroll_speed, -1.0, 1.0);
		ImGui::DragFloat4("proj[2]", &projection.m[8], scroll_speed, -1.0, 1.0);
		ImGui::DragFloat4("proj[3]", &projection.m[12], scroll_speed, -1.0, 1.0);
	
		mat4 mvp = model * view * projection * screen;

		rBufferSetMemory(&projectionBuffer, sizeof(mat4), &mvp);

		rEngineEndFrame(engine);
	}
	
	return EXIT_SUCCESS;
}

