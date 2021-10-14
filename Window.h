#pragma once
#include "stdafx.h"

class Window
{
public:
	static GLFWwindow &getInstance();
	static void cleanUp();

private:
	Window();
	~Window();
	static void init();

	static GLFWwindow* window;
	static const uint32_t HEIGHT = 600;
	static const uint32_t WIDTH = 800;
};
