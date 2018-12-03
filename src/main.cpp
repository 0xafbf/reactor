
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

	rWindow window(engine, "My Greatest Window", 1024, 1024); //name, size
	
	rGraphicsPipeline pipeline(engine, "shaders/basic.vert.spv", "shaders/basic.frag.spv");
	
	rGeometry geometry(engine, "content/proj.obj");
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
	var modelProjection = mat4(1.0);
	var view = mat4(1.0);
	var projection = mat4(1.0);
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

	//projection(0, 0) = 1.0 / far;

	struct { float yaw; float pitch; float distance; } orbit;
	orbit = { 0, 0, 5.0 };
#define deg * TAU / 360.0
	float angle = 45 deg;

	float near = 1.0f;
	float far = 2.0f;
	float aspect_x = 1.0;
	float aspect_y = 1.0;

	while (rEngineShouldTick(engine))
	{
		rEngineStartFrame(engine);
		
		float scroll_speed = 0.01;
		float limit = 10.0f;
		ImGui::DragFloat3("location", &modelLocation.x, scroll_speed);

		if (ImGui::Button("Reset"))
		{
			model = mat4::location(modelLocation);
		}
		if (ImGui::Button("proj1"))
		{
			near = 1.0f;
			far = 2.0f;
			aspect_x = 1.0;
			aspect_y = 1.0;
		}
		if (ImGui::Button("proj2"))
		{
			near = 3.0f;
			far = 6.0f;
			aspect_x = 0.33333;
			aspect_y = 0.33333;
		}
		model = mat4::location(modelLocation);
		ImGui::DragFloat4("model[0]", &model.m[0], scroll_speed, -limit, limit);
		ImGui::DragFloat4("model[1]", &model.m[4], scroll_speed, -limit, limit);
		ImGui::DragFloat4("model[2]", &model.m[8], scroll_speed, -limit, limit);
		ImGui::DragFloat4("model[3]", &model.m[12], scroll_speed, -limit, limit);

		ImGui::DragFloat("near", &near, scroll_speed);
		ImGui::DragFloat("far", &far, scroll_speed);
		ImGui::DragFloat("aspect_x", &aspect_x, scroll_speed);
		ImGui::DragFloat("aspect_y", &aspect_y, scroll_speed);
		static float fov;
		ImGui::DragFloat("fov", &fov, scroll_speed);

		static bool autoproject = true;
		ImGui::Checkbox("autoproject", &autoproject);
		if (autoproject) {
			modelProjection = mat4::cam_perspective(fov, aspect_x/aspect_y, mat4::ASPECT_VERTICAL, near, far);
		}

		static float persp_level = 1.0f;
		ImGui::DragFloat("persp_level", &persp_level, 0.01);

		modelProjection = lerp(mat4(1.), modelProjection, persp_level);


		ImGui::DragFloat4("modelProjection[0]", &modelProjection.m[0], scroll_speed, -limit, limit);
		ImGui::DragFloat4("modelProjection[1]", &modelProjection.m[4], scroll_speed, -limit, limit);
		ImGui::DragFloat4("modelProjection[2]", &modelProjection.m[8], scroll_speed, -limit, limit);
		ImGui::DragFloat4("modelProjection[3]", &modelProjection.m[12], scroll_speed, -limit, limit);
	
		
		// CAMARA
		var io = ImGui::GetIO();
		if (io.MouseDown[0] && !io.WantCaptureMouse)
		{
			let delta = io.MouseDelta;
			let speed = 0.3f;
			orbit.yaw += delta.x * -speed;
			orbit.pitch += delta.y * speed;
		}
		ImGui::DragFloat3("cam_orbit", &orbit.yaw);
		view = mat4::orbit(orbit.distance, orbit.pitch, orbit.yaw);

		////
		ImGui::DragFloat("camfov", &angle, 1 deg);
		projection = mat4::cam_perspective(angle, float(window.width)/float(window.height));
	
		mat4 mvp = model * modelProjection * view * projection * screen;

		rBufferSetMemory(&projectionBuffer, sizeof(mat4), &mvp);

		rEngineEndFrame(engine);
	}
	
	return EXIT_SUCCESS;
}

