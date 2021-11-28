#include "Window.h"

GLFWwindow* Window::window = nullptr;
const char* Window::windowTitle = "Project";

GLFWwindow& Window::getInstance()
{
	if (window == nullptr)
	{
		init();
	}	

	return *window;
}

Window::~Window()
{
}

void Window::init()
{
	glfwInit();

	// Don't create an OpenGL context as we're working with Vulkan
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	window = glfwCreateWindow(WIDTH, HEIGHT, windowTitle, nullptr, nullptr);
}

void Window::cleanUp()
{
	glfwDestroyWindow(&Window::getInstance());
	glfwTerminate();
	log("GLFW resources cleaned up.");
}

const char* Window::getWindowTitle()
{
	return windowTitle;
}
