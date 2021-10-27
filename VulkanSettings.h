#pragma once
#include "stdafx.h"
#include <algorithm>
#include <cstdint> // UINT32_MAX
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <optional>
#include <set>
#include <unordered_map>

#include "Window.h"

#ifdef NDEBUG
const bool ENABLE_VALIDATION_LAYERS = false;
#else
const bool ENABLE_VALIDATION_LAYERS = true;
#endif

class VulkanSettings
{
public:
	VulkanSettings();
	~VulkanSettings();

private:
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete()
		{
			return graphicsFamily.has_value()
				&& presentFamily.has_value();
		}
	};

	struct SwapChainSupportDetails
	{
		// Information regarding the images the physical device supports rendering to (number supported for swapchain, min/max width and height)
		VkSurfaceCapabilitiesKHR capabilities;
		
		// Physical device's supported pixel format, color space, etc.
		std::vector<VkSurfaceFormatKHR> formats;

		// Types of rendering to swapchain supported (queue, mailbox, etc.)
		std::vector<VkPresentModeKHR> presentModes;
	};

	const std::vector<const char*> validationLayers =
	{
		"VK_LAYER_KHRONOS_validation"
	};

	const std::vector<const char*> deviceExtensions =
	{
		 VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	void init();
	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
	void createInstance();
	bool checkValidationLayerSupport();
	std::vector<const char*> getRequiredExtensions();
	void setupDebugMessenger();
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void cleanUp();
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);
	void pickPhysicalDevice();
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	bool isPhysicalDeviceSuitable(VkPhysicalDevice device);
	void createLogicalDevice();
	void createSurface();
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	void createSwapChain();
	void createImageViews();
	void createGraphicsPipeline();
	static std::vector<char> readFile(const std::string& filename);
	VkShaderModule createShaderModule(const std::vector<char>& code);
	void createRenderPass();

private:
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice logicalDevice;
	
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkSurfaceKHR surface;

	VkSwapchainKHR swapchain;
	std::vector<VkImage> swapchainImages;
	VkFormat swapchainImageFormat;
	// Defines the size of the images that can be rendered to the window handle
	VkExtent2D swapchainExtent;

	std::vector<VkImageView> swapchainImageViews;

	VkRenderPass renderPass;
	VkPipelineLayout pipelineLayout;
};
