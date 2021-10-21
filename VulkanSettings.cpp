#include "VulkanSettings.h"

VulkanSettings::VulkanSettings()
{
	init();
}

VulkanSettings::~VulkanSettings()
{
	cleanUp();
}

void VulkanSettings::init()
{
	createInstance();
	setupDebugMessenger();
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	createSwapChain();
	createImageViews();
	createGraphicsPipeline();
}

VkResult VulkanSettings::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	// Retrieves the address of the extension function to create the VkDebugUtilsMessengerEXT object
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

/*
* Cleanup and de-allocate the messenger
*/
void VulkanSettings::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		func(instance, debugMessenger, pAllocator);
	}
}

void VulkanSettings::createInstance()
{
	// Check if required validation layers are available
	if (ENABLE_VALIDATION_LAYERS && !checkValidationLayerSupport())
	{
		throw std::runtime_error("Required validation layers not available!");
	}

	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	// Tells Vulkan driver which global extensions and validation layers will be used
	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	// Acquire extensions from glfw necessary to interface with the Vulkan API
	auto extensions = getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

	// Use validation layers if they're enabled
	if (ENABLE_VALIDATION_LAYERS)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else
	{
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create instance!");
	}
}

bool VulkanSettings::checkValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers)
	{
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
		{
			return false;
		}
	}

	return true;
}

/*
	* Get required extensions according to whether validation layers are enabled or
	*/
std::vector<const char*> VulkanSettings::getRequiredExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (ENABLE_VALIDATION_LAYERS)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

void VulkanSettings::setupDebugMessenger()
{
	if (!ENABLE_VALIDATION_LAYERS)
	{
		return;
	}

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populateDebugMessengerCreateInfo(createInfo);

	// Debug messenger is specific to the Vulkan instance and its validation layers
	if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to setup debug messenger for configuring validation layer logging!");
	}
}

void VulkanSettings::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

	createInfo.messageSeverity = 
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

	createInfo.messageType = 
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

	createInfo.pfnUserCallback = debugCallback;
}

void VulkanSettings::cleanUp()
{
	for (auto imageView : swapchainImageViews)
	{
		vkDestroyImageView(logicalDevice, imageView, nullptr);
	}
	vkDestroySwapchainKHR(logicalDevice, swapchain, nullptr);
	vkDestroyDevice(logicalDevice, nullptr);

	if (ENABLE_VALIDATION_LAYERS)
	{
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	}

	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);

	std::cout << "Vulkan instance resources cleaned up." << std::endl;
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanSettings::debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	std::cerr << "Validation Layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

/*
* Select a graphics card that supports the necessary features
*/
void VulkanSettings::pickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0)
	{
		throw std::runtime_error("Failed to find GPU with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> gpuDevices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, gpuDevices.data());

	std::vector<VkPhysicalDevice> gpuSelections = std::vector<VkPhysicalDevice>();

	for (const auto& gpu : gpuDevices)
	{
		if (isPhysicalDeviceSuitable(gpu))
		{
			gpuSelections.push_back(gpu);
		}
	}

	if (gpuSelections.empty())
	{
		throw std::runtime_error("Failed to find a suitable GPU!");
	}
	else
	{
		std::cout << "~ Select Physical Graphics Device ~" << std::endl;
		std::unordered_map<int, VkPhysicalDevice> physicalDeviceMap = std::unordered_map<int, VkPhysicalDevice>();
		int idx = 1;
		for (std::vector<VkPhysicalDevice>::iterator gpuIter = gpuSelections.begin(); gpuIter != gpuSelections.end(); gpuIter++)
		{
			physicalDeviceMap.insert(std::pair<int, VkPhysicalDevice>(idx, *gpuIter));

			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(*gpuIter, &deviceProperties);

			std::cout << "[" << idx << "] " << deviceProperties.deviceName << std::endl;
			idx++;
		}

		int selection = 0;
		while (physicalDevice == VK_NULL_HANDLE)
		{
			std::cin >> selection;
			try
			{
				physicalDevice = physicalDeviceMap.at(selection);
			}
			catch (const std::exception& e)
			{
				std::cerr << "Invalid graphics device. Specify a different option." << std::endl;
			}
		}

		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

		std::cout << "Graphics Device Selected: " << deviceProperties.deviceName << std::endl;
	}
}

/*
* Identify required queue families for executing commands
*/
VulkanSettings::QueueFamilyIndices VulkanSettings::findQueueFamilies(VkPhysicalDevice device)
{
	// Currently only require a queue family that supports graphics commands
	QueueFamilyIndices indices;

	std::optional<uint32_t> graphicsFamily;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}

		// Look for queue family that has capability of presenting to window surface
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

		if (presentSupport)
		{
			indices.presentFamily = i;
		}

		if (indices.isComplete())
		{
			break;
		}

		i++;
	}

	return indices;
}

bool VulkanSettings::isPhysicalDeviceSuitable(VkPhysicalDevice device)
{
	QueueFamilyIndices indices = findQueueFamilies(device);

	bool extensionsSupported = checkDeviceExtensionSupport(device);

	// If the physical device supports all the required extensions, ensure it also supports the required swapchain formats and modes
	bool swapChainAdequate = false;
	if (extensionsSupported)
	{
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

/*
* Logical device interfaces with the physical device and denotes which features to use
* Specifies which command queues to create
*/
void VulkanSettings::createLogicalDevice()
{
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = {
		indices.graphicsFamily.value(),
		indices.presentFamily.value()
	};

	// Priority of queue for use in scheduling execution on the command buffer
	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		// Structure defines the number of queues desired from a single queue family (currently only concerned with graphics family)
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	// Query support for texture compression, 64-bit floats, and multi-viewport rendering
	VkPhysicalDeviceFeatures deviceFeatures{};

	// Define the 'createInfo' of the logical device
	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pEnabledFeatures = &deviceFeatures;

	/* Define logical device validation layers
	 * This is for legacy Vulkan support as current versions use the same attributes as the Vulkan instance
	 */
	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();
	if (ENABLE_VALIDATION_LAYERS)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create logical device!");
	}

	// Grab a handle to the specific queue (graphics and presentation)
	vkGetDeviceQueue(logicalDevice, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(logicalDevice, indices.presentFamily.value(), 0, &presentQueue);
}

void VulkanSettings::createSurface()
{
	if (glfwCreateWindowSurface(instance, &Window::getInstance(), nullptr, &surface) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create window surface!");
	}
}

bool VulkanSettings::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
	// Determine whether the device supports all the required extensions
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

VulkanSettings::SwapChainSupportDetails VulkanSettings::querySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;

	// Query for the physical device's supported capabilities (min/max number of images in swap chain, min/max width and height of images, etc.)
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	// Query for the physical device's supported formats (pixel format, color space, etc.)
	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	// Query for the physical device's supported presentation modes
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

VkSurfaceFormatKHR VulkanSettings::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	// Check if the preferred color channel 'SRGB' is available
	for (const auto& availableFormat : availableFormats)
	{
		// The '8' after B, G, R, A represents an 8-bit unsigned int for a total of 32-bits per pixel
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB
			&& availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}

		// If the optimal format isn't available, simply return the first surface format
		return availableFormats[0];
	}
}

VkPresentModeKHR VulkanSettings::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	/* If available, use the MAILBOX swapchain presentation mode as it renders images to the swapchain as they're finished
	 * rendering. It requires greater energy usage but renders images that are as up-to-date as possible
	 * and avoids visual tearing
	*/
	for (const auto& availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return availablePresentMode;
		}
	}

	// If not available (or on mobile) use a standard queue to render images
	return VK_PRESENT_MODE_FIFO_KHR;
}

/*
* Determine the resolution of the window in pixels and bind between the min / max image width / height supported by the capabilities of the physical device
*/
VkExtent2D VulkanSettings::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	// Swap extent is the resolution of the swapchain
	if (capabilities.currentExtent.width != UINT32_MAX)
	{
		return capabilities.currentExtent;
	}
	else
	{
		int width, height;
		// Query the resolution of the window in pixels
		glfwGetFramebufferSize(&Window::getInstance(), &width, &height);

		VkExtent2D actualExtent =
		{
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		// Bind the resolution (pixels) against the min and max supported by the image extent
		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

void VulkanSettings::createSwapChain()
{
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

	// Specify the minimum number of images for the swap chain to render
	// Add one to the number of images to prevent waiting on the driver to run internal operations before we can acquire another image to render to
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

	// Don't exceed the maximum number of images (0 means no maximum)
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1; // Amount of layers each image consists of (always one unless developing for 3D)
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // Operation types images in the swapchain will be used for

	// Specify how to handle the swapchain images that will be used across multiple queue families
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	if (indices.graphicsFamily != indices.presentFamily)
	{
		// Image can be used across multiple queue families without explicit ownership transfers
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		// Specify number and indices of queues that will operate with the image
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		// Image is owned by a single queue at a time (graphics queue uses it before transferring to presentation queue)
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	// Specify the transformation to be applied to the image (currentTransform for none) such as a 90-degree rotation, etc.
	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;

	// Specify the alpha channel (used for blending with other windows in the system)
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	
	createInfo.presentMode = presentMode;

	// If set to 'true', we don't care about the color of pixels that are obscured
	createInfo.clipped = VK_TRUE;

	// Maintain a pointer to the previous swapchain as a new one needs to be created from scratch if the current one becomes invalid (i.e. window is resized, etc.)
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr, &swapchain) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create swapchain!");
	}

	// Query for and resize handle to swapchain's images according to the maximum number of images capable
	vkGetSwapchainImagesKHR(logicalDevice, swapchain, &imageCount, nullptr);
	swapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(logicalDevice, swapchain, &imageCount, swapchainImages.data());

	swapchainImageFormat = surfaceFormat.format;
	swapchainExtent = extent;
}

/*
* Create an ImageView for each image in the swapchain. Each ImageView defines how the specific image should be 'viewed' (plain 2D, depth, texture mapped, etc.)
*/
void VulkanSettings::createImageViews()
{
	swapchainImageViews.resize(swapchainImages.size());

	for (size_t i = 0; i < swapchainImages.size(); i++)
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapchainImages[i];

		// Specify how the image data should be interpreted
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapchainImageFormat;

		// 'Swizzle' color channels for image (specify monochrome if desired, here is default color mapping)
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		// Describe what the image's purpose is and which part of the image should be accessed
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;

		// If developing a stereographic 3D application, swapchain images would have multiple layers
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(logicalDevice, &createInfo, nullptr, &swapchainImageViews[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create image views!");
		}
	}
}

void VulkanSettings::createGraphicsPipeline()
{
	auto vertShaderCode = readFile("Shaders/vert.spv");
	auto fragShaderCode = readFile("Shaders/frag.spv");

	// Shader bytecode is converted to machine code upon creation of the graphics pipeline
	// This allows us to create the modules as local variables rather than class as we'll destroy them afterwards
	VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] =
	{
		vertShaderStageInfo,
		fragShaderStageInfo
	};

	vkDestroyShaderModule(logicalDevice, fragShaderModule, nullptr);
	vkDestroyShaderModule(logicalDevice, vertShaderModule, nullptr);
}

std::vector<char> VulkanSettings::readFile(const std::string& filename)
{
	// ate specifies reading at the end of the file
	// binary states the file should be read as a binary file
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open file!");
	}

	// Use 'ate' to begin at the end of the file in order to allocate the buffer to be the size of the file's contents
	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	// Return to the beginning of the file and read all the bytes at once
	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();
	return buffer;
}

VkShaderModule VulkanSettings::createShaderModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();

	// Cast pointer from char* to uint32_t* using reinterpret cast
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create shader module!");
	}

	return shaderModule;
}
