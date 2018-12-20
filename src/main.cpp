
#include "reactor.h"
#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )


int main()
{

	var engine = rEngine("My Great App");
	var window = rWindow(engine, "My Greatest Window", 1024, 1024); 
	var geometry = rGeometry(engine, "content/cube.obj");
	var pipeline = rPipeline(engine, "shaders/basic.vert.spv", "shaders/basic.frag.spv", geometry);
	
	// scene is just a container to auto draw stuff
	rScene scene;
	scene.primitives.push_back(&pipeline);
	window.scene = &scene;
	
	var transform = rTransform();
	var camera = rOrbitCamera();
	var projection = mat4(0.);
	var projectionBuffer = rBuffer(engine, &projection, sizeof(mat4));
	rPipelineUpdateDescriptorSets(pipeline, { { 0, 0, projectionBuffer } });


	var image = rImage(engine, "content/uv.png");
	rPipelineUpdateDescriptorSets(pipeline, { { 0, 1, image} });
	// apparently this is needed to avoid a vulkan warning
	rPipelineUpdateDescriptorSets(pipeline, { { 0, 2, image} });

	while ( rEngineStartFrame(engine))
	{
		rDebug(transform, "myTransform");
		let model = rTransformMatrix(transform);

		// CAMARA
		rCameraTick(camera);
		float aspect_ratio = float(window.width) / float(window.height);
		projection = rCameraProject(camera, model, aspect_ratio) * mat4::screen();
		rBufferSync(projectionBuffer);

		rEngineEndFrame(engine);
	}
	
	return 0;
}

