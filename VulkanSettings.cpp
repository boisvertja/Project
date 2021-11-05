#include "VulkanSettings.h"

VulkanSettings::VulkanSettings()
{
	init();
}

VulkanSettings::~VulkanSettings()
{
	cleanUp();
}

VkDevice VulkanSettings::getLogicalDevice() const
{
	return logicalDevice;
}

void VulkanSettings::init()
{
	createInstance();
	setupDebugMessenger();
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
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
	log("Vulkan instance created.");

	// Log the Vulkan API version used
	uint32_t vkInstanceVersion = appInfo.apiVersion;
	vkEnumerateInstanceVersion(&vkInstanceVersion);

	uint32_t major = VK_VERSION_MAJOR(vkInstanceVersion);
	uint32_t minor = VK_VERSION_MINOR(vkInstanceVersion);
	uint32_t patch = VK_VERSION_PATCH(vkInstanceVersion);

	log("Vulkan API Version: " << major << "." << minor << "." << patch);
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
	log("DebugMessenger created.");
}

void VulkanSettings::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

	createInfo.messageSeverity = 
		/*VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT*/
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

	createInfo.messageType = 
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

	createInfo.pfnUserCallback = debugCallback;
}

void VulkanSettings::cleanUp()
{
	vkDestroySurfaceKHR(instance, surface, nullptr);
	log("Surface destroyed.");

	vkDestroyDevice(logicalDevice, nullptr);
	log("Logical device destroyed.");

	if (ENABLE_VALIDATION_LAYERS)
	{
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		log("DebugMessenger destroyed.");
	}

	vkDestroyInstance(instance, nullptr);
	log("Vulkan instance destroyed.");
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanSettings::debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	log("Validation Layer: " << pCallbackData->pMessage);

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
		log("~ Select Physical Graphics Device ~");
		std::unordered_map<int, VkPhysicalDevice> physicalDeviceMap = std::unordered_map<int, VkPhysicalDevice>();
		int idx = 1;
		for (std::vector<VkPhysicalDevice>::iterator gpuIter = gpuSelections.begin(); gpuIter != gpuSelections.end(); gpuIter++)
		{
			physicalDeviceMap.insert(std::pair<int, VkPhysicalDevice>(idx, *gpuIter));

			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(*gpuIter, &deviceProperties);

			log("[" << idx << "] " << deviceProperties.deviceName);
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
				log("Invalid graphics device. Specify a different option." << e.what());
			}
		}

		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

		log("Graphics Device Selected: " << deviceProperties.deviceName);
	}
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

/*
* Logical device interfaces with the physical device and denotes which features to use
* Specifies which command queues to create
*/
void VulkanSettings::createLogicalDevice()
{
	queueIndices = findQueueFamilies(physicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = {
		queueIndices.graphicsFamily.value(),
		queueIndices.presentFamily.value()
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
	log("Logical device created.");
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

void VulkanSettings::createSurface()
{
	if (glfwCreateWindowSurface(instance, &Window::getInstance(), nullptr, &surface) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create window surface!");
	}
	log("Window surface created.");
}
