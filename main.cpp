#include "VulkanSettings.h"
#include "Window.h"

int main()
{
	Window window = Window();
	VulkanSettings vkSettings = VulkanSettings();

	try
	{
		while (!glfwWindowShouldClose(window.getWindow()))
		{
			glfwPollEvents();
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
