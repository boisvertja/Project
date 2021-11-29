#pragma once
#include "stdafx.h"

namespace VulkanProject
{
	class Window
	{
	public:
		static GLFWwindow& getInstance();
		static void cleanUp();
		static const char* getWindowTitle();

	private:
		Window();
		~Window();
		static void init();

		static GLFWwindow* window;
		static const uint32_t HEIGHT = 600;
		static const uint32_t WIDTH = 800;
		static const char* windowTitle;
	};
}
