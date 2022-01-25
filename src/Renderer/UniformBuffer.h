#pragma once
#include "Buffer.h"
#include <chrono>

namespace VulkanProject
{
	class UniformBuffer : Buffer
	{
	public:
		UniformBuffer() = default;
		~UniformBuffer() = default;

		void update(VkExtent2D swapchainExtent);

		struct UniformBufferObject
		{
			glm::mat4 model;
			glm::mat4 view;
			glm::mat4 proj;
		};

		friend class Renderer;
	};
}
