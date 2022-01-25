#pragma once
#include "Buffer.h"

namespace VulkanProject
{
	class IndexBuffer : Buffer
	{
	public:
		IndexBuffer() = default;

		IndexBuffer(std::vector<uint32_t> indices, VkQueue graphicsQueue, VkCommandPool commandPool);
		~IndexBuffer();

		friend class Renderer;
	};
}
