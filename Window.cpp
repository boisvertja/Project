#include "Window.h"

Window::Window()
{
	init();
}

Window::~Window()
{
	cleanUp();
}

void Window::init()
{
	glfwInit();

	// Don't create an OpenGL context as we're working with Vulkan
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(WIDTH, HEIGHT, "Project", nullptr, nullptr);
}

GLFWwindow* Window::getWindow() const
{
	return window;
}

void Window::cleanUp()
{
	glfwDestroyWindow(window);
	glfwTerminate();
	std::cout << "GLFW resources cleaned up." << std::endl;
}
