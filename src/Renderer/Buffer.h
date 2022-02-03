#pragma once
#include "../Core/VulkanSettings.h"

namespace VulkanProject
{
	class Buffer
	{
	public:
		Buffer() = default;
		~Buffer() = default;
		static void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		static void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkQueue graphicsQueue, VkCommandPool commandPool);
		static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		static VkCommandBuffer createCommandBuffer(VkCommandPool commandPool);
		static void beginCommandBuffer(VkCommandBuffer& commandBuffer, VkCommandBufferUsageFlagBits usageFlags);
		static void endCommandBuffer(VkCommandBuffer& commandBuffer, VkQueue graphicsQueue, VkCommandPool commandPool);

		VkBuffer& getBuffer();
		VkDeviceMemory& getBufferMemory();

	protected:
		VkBuffer bufferHandle;
		VkDeviceMemory bufferMemory;
	};
}
