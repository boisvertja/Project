#pragma once
#include "stdafx.h"

class Window
{
public:
	Window();
	~Window();
	GLFWwindow* getWindow() const;

private:
	void init();
	void cleanUp();

	GLFWwindow* window;
	static const uint32_t HEIGHT = 600;
	static const uint32_t WIDTH = 800;
};
