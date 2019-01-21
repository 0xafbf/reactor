
#include "reactor.h"

int main()
{
	var engine = rEngine("My Great App");
	var window = rWindow(engine, "My Greatest Window", 512, 512);
	var pipeline = rPipeline(engine, "shaders/basic.slang");
	
	var geometry = rGeometry(engine, "content/cube.obj");

	rState state;
	state.pipeline = &pipeline;
	state.geometry = &geometry;
	state.descriptor_sets.push_back(rDescriptorSet(engine.device, engine.descriptor_pool, pipeline.descriptor_set_layouts[0]));

	rScene scene;
	scene.primitives.push_back(&state);
	window.scene = &scene;
	
	// per instance
	var transform = rTransform();
	var transform_mat = rTransformMatrix(transform);
	var transform_buffer = rBuffer(engine, &transform_mat, sizeof(mat4));
	rStateSetDescriptor(engine.device, state, 0, transform_buffer);

	var image = rImage(engine, "content/uv.png");
	rStateSetDescriptor(engine.device, state, 2, image, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
	rStateSetDescriptor(engine.device, state, 3, image, VK_DESCRIPTOR_TYPE_SAMPLER);

	// per scene
	var camera = rOrbitCamera();
	var projection = mat4(0.);
	var projection_buffer = rBuffer(engine, &projection, sizeof(mat4));
	rStateSetDescriptor(engine.device, state, 1, projection_buffer);

	var transf2 = rTransform();

	while ( rEngineStartFrame(engine))
	{

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




#include <windows.h>

int  WinMain(
   HINSTANCE hInstance,
   HINSTANCE hPrevInstance,
   LPSTR     lpCmdLine,
   int       nCmdShow
){
	return main();
}