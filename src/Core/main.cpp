#include "../Renderer/Renderer.h"

using namespace VulkanProject;

int main()
{
	try
	{
		GLFWwindow& window = Window::getInstance();
		Renderer renderer = Renderer();
		
		while (!glfwWindowShouldClose(&window))
		{
			glfwPollEvents();
			renderer.drawFrame();
			renderer.calculateFPS();
		}

		vkDeviceWaitIdle(VulkanSettings::getInstance()->getLogicalDevice());

		LOG("\nCleaning resources...");
		LOG("=========================");
		Window::cleanUp();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
#pragma warning(pop)
