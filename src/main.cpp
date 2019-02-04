
#include "reactor.h"




void rDebug(rGraphicsPipeline& graphics_pipeline) {
	  
  auto fff = (int*)(&graphics_pipeline.rasterizer.frontFace);
  if (ImGui::Combo("Front face", fff, "counter clockwise\0clockwise\0"))
	{
	  INFO("hey whats up %d", *fff);
	}
  
}

int main()
{
	auto engine = rEngine("My Great App");
	auto window = rWindow(engine, "My Greatest Window", 512, 512);
	auto pipeline = rPipeline(engine, "shaders/basic.slang");
	
	auto geometry = rGeometry(engine, "content/cube.obj");

	rState state;
	state.pipeline = &pipeline;
	state.geometry = &geometry;
	state.descriptor_sets.push_back(rDescriptorSet(engine.device, engine.descriptor_pool, pipeline.descriptor_set_layouts[0]));

	rScene scene;
	scene.primitives.push_back(&state);
	window.scene = &scene;
	
	// per instance
	auto transform = rTransform();
	auto transform_mat = rTransformMatrix(transform);
	auto transform_buffer = rBuffer(engine, &transform_mat, sizeof(mat4));
	rStateSetDescriptor(engine.device, state, 0, transform_buffer);

	auto image = rImage(engine, "content/uv.png");
	rStateSetDescriptor(engine.device, state, 2, image, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
	rStateSetDescriptor(engine.device, state, 3, image, VK_DESCRIPTOR_TYPE_SAMPLER);

	// per scene
	auto camera = rOrbitCamera();
	auto projection = mat4(0.);
	auto projection_buffer = rBuffer(engine, &projection, sizeof(mat4));
	rStateSetDescriptor(engine.device, state, 1, projection_buffer);

	auto transf2 = rTransform();

	while ( rEngineStartFrame(engine))
	{
		rDebug(pipeline);
		rDebug(transform, "transform");
		transform_mat = rTransformMatrix(transform) * rTransformMatrix(transf2);
		rDebug(transform_mat, "matrix");
		rBufferSync(transform_buffer);

		rCameraTick(camera);
		float aspect_ratio = float(window.width) / float(window.height);
		projection = rCameraProject(camera, aspect_ratio) * mat4::screen();
		rBufferSync(projection_buffer);

		rEngineEndFrame(engine);
	}
	
	return 0;
}
