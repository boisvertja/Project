#pragma once
#include <array>
#include "Shader.h"
#include "VulkanSettings.h"

class Renderer
{
public:
	Renderer();
	~Renderer();
	void drawFrame();
	VulkanSettings getVulkanSettings() const;

private:
	void init();
	void cleanUp();
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	void createSwapChain();
	void createImageViews();
	void createGraphicsPipeline();
	void createRenderPass();
	void grabHandlesForQueues();
	void createFramebuffers();
	void createCommandPool();
	void createCommandBuffers();
	void createSyncObjects();
	void recreateSwapchain();
	void cleanUpSwapchain();
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
	void createVertexBuffer();
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

private:
	VkQueue graphicsQueue;
	VkQueue presentQueue;

	VkSwapchainKHR swapchain;
	std::vector<VkImage> swapchainImages;
	VkFormat swapchainImageFormat;
	// Defines the size of the images that can be rendered to the window handle
	VkExtent2D swapchainExtent;

	std::vector<VkImageView> swapchainImageViews;
	std::vector<VkFramebuffer> swapchainFramebuffers;

	VkRenderPass renderPass;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;
	VkCommandPool commandPool;
	std::vector<VkCommandBuffer> commandBuffers;

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	std::vector<VkFence> imagesInFlight;
	const int MAX_FRAMES_IN_FLIGHT = 2;
	bool framebufferResized;
	size_t currentFrame = 0;

	VulkanSettings vkSettings = VulkanSettings();

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

	std::vector<Vertex> vertices =
	{
		{{0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}},
		{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
		{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
	};

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
};
