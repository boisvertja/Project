#pragma once
#include "../Core/VulkanSettings.h"

namespace VulkanProject
{
	class Buffer
	{
	public:
		Buffer() = default;
		~Buffer() = default;
		static void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkQueue graphicsQueue, VkCommandPool commandPool);

		VkBuffer& getBuffer();
		VkDeviceMemory& getBufferMemory();

	protected:
		static void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

		VkBuffer bufferHandle;
		VkDeviceMemory bufferMemory;
	};
}
