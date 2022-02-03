#include "Buffer.h"

namespace VulkanProject
{
	void Buffer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
    {
		VkDevice logicalDevice = VulkanSettings::getInstance()->getLogicalDevice();
		VkPhysicalDevice physicalDevice = VulkanSettings::getInstance()->getPhysicalDevice();

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(logicalDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create buffer!");
		}

		// Assign memory to the buffer
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(logicalDevice, buffer, &memRequirements);

		// Allocate the memory
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate buffer memory!");
		}

		// Associate the memory to the vertex buffer handle
		vkBindBufferMemory(logicalDevice, buffer, bufferMemory, 0);
    }

	/*
	* Copy the memory contents of the 'src' buffer to the 'dst' buffer.
	* This requires the usage of a temporary command buffer to execute the memory transfer operation.
	*/
	void Buffer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkQueue graphicsQueue, VkCommandPool commandPool)
	{
		VkDevice logicalDevice = VulkanSettings::getInstance()->getLogicalDevice();

		VkCommandBuffer commandBuffer = createCommandBuffer(commandPool);
		beginCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		// Specify the region of memory to copy from one buffer to the next
		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0; // Optional
		copyRegion.dstOffset = 0; // Optional
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		// Execute the command buffer to complete the transfer
		endCommandBuffer(commandBuffer, graphicsQueue, commandPool);
	}
	
	uint32_t Buffer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDevice physicalDevice = VulkanSettings::getInstance()->getPhysicalDevice();

		// Query information about the available types of memory
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			// Locate a memory type that is suitable for the vertex buffer as well as writing vertex data to the memory
			// Memory type array specifies heap and properties of each type of memory i.e. being able to write to it from the CPU
			// Check if the result of the bitwise AND is not just non-zero, but equal to the desired property's bit field
			// (If there is a memory type suitable for the buffer that also has all the properties we need)
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		throw std::runtime_error("Unable to find memory type suitable for the vertex buffer!");
	}

	VkBuffer& Buffer::getBuffer()
	{
		return bufferHandle;
	}

	VkDeviceMemory& Buffer::getBufferMemory()
	{
		return bufferMemory;
	}

	VkCommandBuffer Buffer::createCommandBuffer(VkCommandPool commandPool)
	{
		VkDevice logicalDevice = VulkanSettings::getInstance()->getLogicalDevice();

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		if (vkAllocateCommandBuffers(logicalDevice, &allocInfo, &commandBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate command buffer!");
		}

		return commandBuffer;
	}

	void Buffer::beginCommandBuffer(VkCommandBuffer& commandBuffer, VkCommandBufferUsageFlagBits usageFlags)
	{
		VkDevice logicalDevice = VulkanSettings::getInstance()->getLogicalDevice();

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		// Optional - specifies how we're going to use the command buffer i.e. one-time submit, per renderPass, etc.
		beginInfo.flags = usageFlags;
		beginInfo.pInheritanceInfo = nullptr; // Optional - only relevant for secondary command buffers (specifies which state to inherit from calling primary command buffer)

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to begin recording command buffer!");
		}
	}

	/*
	 * Execute the command buffer to complete the transfer
	*/
	void Buffer::endCommandBuffer(VkCommandBuffer& commandBuffer, VkQueue graphicsQueue, VkCommandPool commandPool)
	{
		VkDevice logicalDevice = VulkanSettings::getInstance()->getLogicalDevice();

		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphicsQueue);

		vkFreeCommandBuffers(logicalDevice, commandPool, 1, &commandBuffer);
	}
}
