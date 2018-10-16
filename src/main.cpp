
#include "engine.h"

int main() {

	rEngine engine;
	engine.name = "My Greatest App";

	rStartEngine(&engine);

	rWindow window;
	window.width = 800;
	window.height = 600;
	window.name = "My Greatest Window";
	rCreateWindow(&engine, &window);
	
	rCreatePipeline();
	while (!glfwWindowShouldClose(window.glfwWindow)) {
		glfwPollEvents();
	}
	
	rDestroyEngine(&engine);
	
	return EXIT_SUCCESS;
}

