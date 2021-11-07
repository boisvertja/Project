#pragma once
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
};
