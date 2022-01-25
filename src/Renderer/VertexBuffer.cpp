#include "VertexBuffer.h"

namespace VulkanProject
{
	VertexBuffer::VertexBuffer(std::vector<Vertex> vertices, VkQueue graphicsQueue, VkCommandPool commandPool)
	{
		VkDevice logicalDevice = VulkanSettings::getInstance()->getLogicalDevice();

		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

		// Create a staging buffer. This will allow us to load the vertex information into this CPU-accessible buffer and then 'transfer' the memory
		// contents to a buffer on the GPU (not visible from CPU) that utilizes high performance memory
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		Buffer::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		// Copy the vertex data to the buffer. This requires mapping the buffer memory into CPU accessible memory
		void* stagingData;
		vkMapMemory(logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &stagingData);
		memcpy(stagingData, vertices.data(), (size_t)bufferSize);
		vkUnmapMemory(logicalDevice, stagingBufferMemory);
		
		Buffer::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, bufferHandle, bufferMemory);
		copyBuffer(stagingBuffer, bufferHandle, bufferSize, graphicsQueue, commandPool);

		// Cleanup resources after copying the data from the CPU-accessible 'stagingBuffer' to the high performance memory, GPU-accessible buffer
		vkDestroyBuffer(logicalDevice, stagingBuffer, nullptr);
		vkFreeMemory(logicalDevice, stagingBufferMemory, nullptr);
	}

	VertexBuffer::~VertexBuffer()
	{
	}
}
