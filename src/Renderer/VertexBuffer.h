#pragma once
#include <array>
#include "Buffer.h"

namespace VulkanProject
{
	class VertexBuffer : Buffer
	{
	private:
		struct Vertex
		{
			glm::vec2 pos;
			glm::vec3 color;

			/*
			* Describes at which rate to load data from memory throughout the vertices of the shader.
			* Also specifies the number of bytes between data entries (stride)
			*/
			static VkVertexInputBindingDescription getBindingDescription()
			{
				VkVertexInputBindingDescription bindingDescription{};
				bindingDescription.binding = 0;
				bindingDescription.stride = sizeof(Vertex);

				// Move to the next data entry after each vertex
				bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

				return bindingDescription;
			}

			/*
			* Describes how to extract a vertex attribute from a chunk of vertex data originating from a binding description
			*/
			static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions()
			{
				// There are two attributes, description and color, so there are two attribute description structs
				std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

				// Tells Vulkan from which binding the per-vertex data comes
				attributeDescriptions[0].binding = 0;

				// References the 'location' directive of the input in the vertex shader
				attributeDescriptions[0].location = 0;

				// Describes the type of data for the attribute
				attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
				attributeDescriptions[0].offset = offsetof(Vertex, pos);

				attributeDescriptions[1].binding = 0;
				attributeDescriptions[1].location = 1;
				attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
				attributeDescriptions[1].offset = offsetof(Vertex, color);

				return attributeDescriptions;
			}
		};

	public:
		VertexBuffer() = default;

		VertexBuffer(std::vector<Vertex> vertices, VkQueue graphicsQueue, VkCommandPool commandPool);
		~VertexBuffer();

		friend class Renderer;
	};
}
