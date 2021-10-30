#include "Renderer.h"

int main()
{
	try
	{
		GLFWwindow& window = Window::getInstance();
		Renderer renderer = Renderer();

		while (!glfwWindowShouldClose(&window))
		{
			glfwPollEvents();
		}

		log("\nCleaning resources...");
		log("=========================");
		Window::cleanUp();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
