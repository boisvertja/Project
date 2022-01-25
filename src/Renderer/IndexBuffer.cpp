#include "../Renderer/IndexBuffer.h"

namespace VulkanProject
{
    IndexBuffer::IndexBuffer(std::vector<uint32_t> indices, VkQueue graphicsQueue, VkCommandPool commandPool)
    {
		VkDevice logicalDevice = VulkanSettings::getInstance()->getLogicalDevice();

		VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		Buffer::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* stagingBufferData;
		vkMapMemory(logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &stagingBufferData);
		memcpy(stagingBufferData, indices.data(), (size_t)bufferSize);
		vkUnmapMemory(logicalDevice, stagingBufferMemory);

		Buffer::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, bufferHandle, bufferMemory);

		copyBuffer(stagingBuffer, bufferHandle, bufferSize, graphicsQueue, commandPool);

		vkDestroyBuffer(logicalDevice, stagingBuffer, nullptr);
		vkFreeMemory(logicalDevice, stagingBufferMemory, nullptr);
    }

    IndexBuffer::~IndexBuffer()
    {
    }
}
