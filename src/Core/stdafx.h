#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>

#ifdef _DEBUG
#define log(x) std::cout << x << std::endl
#else
#define log(x)
#endif
