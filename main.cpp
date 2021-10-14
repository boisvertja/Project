#include "VulkanSettings.h"
#include "Window.h"

int main()
{
	try
	{
		GLFWwindow& window = Window::getInstance();
		VulkanSettings vkSettings = VulkanSettings();

		while (!glfwWindowShouldClose(&window))
		{
			glfwPollEvents();
		}
		Window::cleanUp();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
