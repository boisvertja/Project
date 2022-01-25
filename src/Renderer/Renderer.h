#pragma once
#include "Shader.h"
#include "IndexBuffer.h"
#include "UniformBuffer.h"
#include "VertexBuffer.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES

namespace VulkanProject
{
	class Renderer
	{
	public:
		Renderer();
		~Renderer();
		void drawFrame();
		void calculateFPS();

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
		void createDescriptorSetLayout();
		void createVertexBuffers();
		void createIndexBuffers();
		void createUniformBuffers();
		void createDescriptorPool();
		void createDescriptorSets();

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
		VkDescriptorSetLayout descriptorSetLayout;
		VkPipelineLayout pipelineLayout;
		VkPipeline graphicsPipeline;
		VkCommandPool commandPool;
		std::vector<VkCommandBuffer> commandBuffers;

		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;
		std::vector<VkFence> imagesInFlight;
		const int32_t MAX_FRAMES_IN_FLIGHT = 2;
		bool framebufferResized;
		size_t currentFrame = 0;
		VulkanSettings* vkSettings = VulkanSettings::getInstance();

		const std::vector<VertexBuffer::Vertex> vertices =
		{
			{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},
			{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
		};

		const std::vector<uint32_t> indices =
		{
			0, 1, 2, 2, 3, 0
		};

		VertexBuffer vertexBuffer;
		IndexBuffer indexBuffer;
		std::vector<UniformBuffer> uniformBuffers;

		VkDescriptorPool descriptorPool;
		std::vector<VkDescriptorSet> descriptorSets;
	};
}
