#include "../Renderer/Renderer.h"

int main()
{
	try
	{
		GLFWwindow& window = VulkanProject::Window::getInstance();
		VulkanProject::Renderer renderer = VulkanProject::Renderer();

		while (!glfwWindowShouldClose(&window))
		{
			glfwPollEvents();
			renderer.drawFrame();
			renderer.calculateFPS();
		}

		vkDeviceWaitIdle(renderer.getVulkanSettings().getLogicalDevice());

		log("\nCleaning resources...");
		log("=========================");
		VulkanProject::Window::cleanUp();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
