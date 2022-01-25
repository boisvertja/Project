#include "UniformBuffer.h"

namespace VulkanProject
{
	/*
	* Ensure the geometry rotates 90 degrees every second regardless of frame rate
	*/
	void UniformBuffer::update(VkExtent2D swapchainExtent)
	{
		VkDevice logicalDevice = VulkanSettings::getInstance()->getLogicalDevice();

		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		// Define the transformation to perform
		// glm::rotate takes an existing transformation (glm::mat4(..)) - identity matrix
			// rotation angle
			// rotation axis
		int numberOfSecondsPerRotation = 3;
		UniformBufferObject ubo{};
		ubo.model = glm::rotate(glm::mat4(1.0f), numberOfSecondsPerRotation * time * glm::radians(90.0f), glm::vec3(0.5f, 0.0f, 1.0f));

		// Looking at geometry from above at a 45-degree angle
		// glm::lookAt takes the eye position, center position, and up axis as parameters
		ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		// Use a perspective projection with a 45-degree vertical field of view
		// Also takes the aspect ratio and near / far planes
		ubo.proj = glm::perspective(glm::radians(45.0f), swapchainExtent.width / (float)swapchainExtent.height, 0.1f, 10.0f);

		// GLM was originally designed for OpenGL (inverted Y-axis) - multiply projection scaling by -1 to correct for Vulkan otherwise the image will appear upside-down
		ubo.proj[1][1] *= -1;

		// Copy the data in the UniformBufferObject to the current uniform buffer
		void* data;
		vkMapMemory(logicalDevice, bufferMemory, 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(logicalDevice, bufferMemory);
	}
}
