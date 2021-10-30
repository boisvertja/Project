#pragma once
#include "Shader.h"
#include "VulkanSettings.h"

class Renderer
{
public:
	Renderer();
	~Renderer();

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

private:
	VkQueue graphicsQueue;
	VkQueue presentQueue;

	VkSwapchainKHR swapchain;
	std::vector<VkImage> swapchainImages;
	VkFormat swapchainImageFormat;
	// Defines the size of the images that can be rendered to the window handle
	VkExtent2D swapchainExtent;

	std::vector<VkImageView> swapchainImageViews;

	VkRenderPass renderPass;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;

	VulkanSettings vkSettings = VulkanSettings();
};
