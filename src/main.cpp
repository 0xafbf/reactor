
#include "engine.h"
#include "window.h"
#include <iostream>
#include "reactor/scene.h"


int main()
{
	rEngine engine("My Great App");

	rWindow window(&engine, "My Greatest Window", 800, 600); //name, size
	rWindow window2(&engine, "My Greatest Window2", 800, 600); //name, size
	
	rPrimitive prim(&engine, "../shaders/vert.spv", "../shaders/frag.spv");
	rScene scene;
	scene.primitives.push_back(&prim);
	window.scene = &scene;
	window2.scene = &scene;
	/*
	while (rEngineShouldTick(&engine))
	{
		glfwPollEvents();
		rEngineTick(&engine);
	}
	*/
	
	rEngineMainLoop(&engine);
	
	return EXIT_SUCCESS;
}

