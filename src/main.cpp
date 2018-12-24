
#include "reactor.h"
#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )


int main()
{

	var engine = rEngine("My Great App");
	var window = rWindow(engine, "My Greatest Window", 1024, 1024); 
	var pipeline = rPipeline(engine, "shaders/basic.slang");
	
	var geometry = rGeometry(engine, "content/cube.obj");
	rPipelineSetGeometry(pipeline, geometry);
	// scene is just a container to auto draw stuff
	rScene scene;
	scene.primitives.push_back(&pipeline);
	window.scene = &scene;
	
	// maybe there is a better way to fit this
	var transform = rTransform();
	var transform_mat = rTransformMatrix(transform);
	var transform_buffer = rBuffer(engine, &transform_mat, sizeof(mat4));
	rPipelineUpdateDescriptorSets(pipeline, { { 0, 0, transform_buffer } });

	var camera = rOrbitCamera();
	var projection = mat4(0.);
	var projectionBuffer = rBuffer(engine, &projection, sizeof(mat4));
	rPipelineUpdateDescriptorSets(pipeline, { { 0, 1, projectionBuffer } });

	var image = rImage(engine, "content/uv.png");
	rPipelineUpdateDescriptorSets(pipeline, { { 0, 2, image} });
	// apparently this is needed to avoid a vulkan warning
	rPipelineUpdateDescriptorSets(pipeline, { { 0, 3, image} });

	while ( rEngineStartFrame(engine))
	{
		// object transform, maybe merge with some object descriptor set stuff
		rDebug(transform, "myTransform");
		transform_mat = rTransformMatrix(transform);
		rBufferSync(transform_buffer);

		// camera, maybe merge with world descriptor set stuff
		rCameraTick(camera);
		float aspect_ratio = float(window.width) / float(window.height);
		projection = rCameraProject(camera, aspect_ratio) * mat4::screen();
		rBufferSync(projectionBuffer);

		rEngineEndFrame(engine);
	}
	
	return 0;
}

