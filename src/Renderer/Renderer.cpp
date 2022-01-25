#include "Renderer.h"

namespace VulkanProject
{
	Renderer::Renderer()
	{
		// Create callback for resizing the framebuffer
		glfwSetWindowUserPointer(&Window::getInstance(), this);
		glfwSetFramebufferSizeCallback(&Window::getInstance(), framebufferResizeCallback);

		init();
	}

	Renderer::~Renderer()
	{
		cleanUp();
	}

	void Renderer::init()
	{
		grabHandlesForQueues();
		createSwapChain();
		createImageViews();
		createRenderPass();
		createDescriptorSetLayout();
		createGraphicsPipeline();
		createFramebuffers();
		createCommandPool();
		createVertexBuffers();
		createIndexBuffers();
		createUniformBuffers();
		createDescriptorPool();
		createDescriptorSets();
		createCommandBuffers();
		createSyncObjects();
	}

	void Renderer::grabHandlesForQueues()
	{
		vkGetDeviceQueue(vkSettings->logicalDevice, vkSettings->queueIndices.graphicsFamily.value(), 0, &graphicsQueue);
		vkGetDeviceQueue(vkSettings->logicalDevice, vkSettings->queueIndices.presentFamily.value(), 0, &presentQueue);
	}

	void Renderer::cleanUp()
	{
		cleanUpSwapchain();

		vkDestroyDescriptorSetLayout(vkSettings->logicalDevice, descriptorSetLayout, nullptr);

		vkDestroyBuffer(vkSettings->logicalDevice, indexBuffer.getBuffer(), nullptr);
		vkFreeMemory(vkSettings->logicalDevice, indexBuffer.getBufferMemory(), nullptr);

		vkDestroyBuffer(vkSettings->logicalDevice, vertexBuffer.getBuffer(), nullptr);
		vkFreeMemory(vkSettings->logicalDevice, vertexBuffer.getBufferMemory(), nullptr);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroySemaphore(vkSettings->logicalDevice, renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(vkSettings->logicalDevice, imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(vkSettings->logicalDevice, inFlightFences[i], nullptr);
		}
		LOG("Semaphores destroyed.");

		vkDestroyCommandPool(vkSettings->logicalDevice, commandPool, nullptr);

		vkSettings->cleanUp();
	}

	void Renderer::createSwapChain()
	{
		VulkanSettings::SwapChainSupportDetails swapChainSupport = vkSettings->querySwapChainSupport(vkSettings->physicalDevice);

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
		createInfo.surface = vkSettings->surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1; // Amount of layers each image consists of (always one unless developing for 3D)
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // Operation types images in the swapchain will be used for

		// Specify how to handle the swapchain images that will be used across multiple queue families
		VulkanSettings::QueueFamilyIndices indices = vkSettings->findQueueFamilies(vkSettings->physicalDevice);
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

		if (vkCreateSwapchainKHR(vkSettings->logicalDevice, &createInfo, nullptr, &swapchain) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create swapchain!");
		}
		LOG("Swapchain created.");

		// Query for and resize handle to swapchain's images according to the maximum number of images capable
		vkGetSwapchainImagesKHR(vkSettings->logicalDevice, swapchain, &imageCount, nullptr);
		swapchainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(vkSettings->logicalDevice, swapchain, &imageCount, swapchainImages.data());

		swapchainImageFormat = surfaceFormat.format;
		swapchainExtent = extent;
	}

	/*
	* Create an ImageView for each image in the swapchain. Each ImageView defines how the specific image should be 'viewed' (plain 2D, depth, texture mapped, etc.)
	*/
	void Renderer::createImageViews()
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

			if (vkCreateImageView(vkSettings->logicalDevice, &createInfo, nullptr, &swapchainImageViews[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create image views!");
			}
		}
		LOG("ImageViews created.");
	}

	/*
	* RenderPass tells Vulkan about the framebuffer attachments used for rendering
	* Specifies how many color / depth buffers there will be, how many samples to use for each of them
	* and how their contents should be handled throughout the rendering operations
	*/
	void Renderer::createRenderPass()
	{
		// Only using a single color buffer attachment represented by one of the images from the swapchain
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = swapchainImageFormat; // Should always match that of the swapchain images
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // Not using multisampling yet so stick to 1 sample

		// These apply to color and depth data
		// What to do with the data in the attachment before and after rendering
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // Clear framebuffer to 'black' before drawing a new frame
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		// These define the same operations but for stencil data (not currently using stencil buffer so don't care)
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		// Images should be transitioned to specific layouts that are suitable for the operation that they're going to be involved in next
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Don't care what previous layout the image was in
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // We want the image to be ready for presentation using swapchain after rendering

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0; // Specifies which attachment to reference by index in the attachment desriptions array
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // Specifies which layout we would like the attachment to have during a subpass that uses this reference

		// Create a new basic subpass that references the attachment(s)
		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		// This implicit subpass refers to the operations that execute right before / after the actual subpass
		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;

		// Index '0' refers to the actual subpass (the first and only one)
		dependency.dstSubpass = 0;

		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;

		// Specify the operations to wait on and the stages in which these operations occur
		// In this case, we want to wait for the swapchain to finish reading the image before we access it, 
		// this can be accomplished by waiting on the color attachment
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(vkSettings->logicalDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create render pass!");
		}
		LOG("RenderPass created.");
	}

	void Renderer::createGraphicsPipeline()
	{
		Shader vertexShader = Shader(Shader::SHADER_TYPE::VERTEX, vkSettings->logicalDevice);
		Shader fragmentShader = Shader(Shader::SHADER_TYPE::FRAGMENT, vkSettings->logicalDevice);

		// Shader bytecode is converted to machine code upon creation of the graphics pipeline
		// This allows us to create the modules as local variables rather than class as we'll destroy them afterwards
		VkShaderModule vertShaderModule = vertexShader.getShaderModule();
		VkShaderModule fragShaderModule = fragmentShader.getShaderModule();

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

		// Describe the format of the vertex data that will be passed to the vertex shader
		// Bindings: Spacing between data and whether the data is per-vertex or per-instance
		// Attribute descriptions: Type of the attributes passed to the vertex shader, which binding to load them from and at which offset
		auto bindingDescription = VertexBuffer::Vertex::getBindingDescription();
		auto attributeDescriptions = VertexBuffer::Vertex::getAttributeDescriptions();

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription; // Optional - point to array of structs that define details for loading vertices
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data(); // Optional - point to array of structs that define details for loading vertices

		// This state describes the geometry to be drawn with the vertices and whether 'primitive restart' should be enabled
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		// If set to 'VK_TRUE', it's possible to break up lines and triangles in the '_STRIP' topology modes in order to optimize usage of the vertex buffer through indices
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		// Viewports and scissors
		// Viewport defines the region of the framebuffer the output will be rendered to
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)swapchainExtent.width;
		viewport.height = (float)swapchainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		// Scissors define in which region the pixels will be stored (any pixels outside the scissor, even if within the viewport, will be discarded)
		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = swapchainExtent;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		// Rasterizer turns vertices from the vertex shader into fragments to be colored by the fragment shader
		// Also performs depth testing, face culling, and scissor test, can additionally be configured for wireframe
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE; // If set to 'VK_TRUE', fragments beyond near/far planes are clamped to them rather than being discarded
		rasterizer.rasterizerDiscardEnable = VK_FALSE; // If set to 'VK_TRUE', then geometry never passes through the rasterizer stage.
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // Determines how fragments are generated for geometry (FILL, LINE (wireframe), POINT) - any other than 'FILL' requires enabling GPU feature
		rasterizer.lineWidth = 1.0f; // In terms of number of fragments
		rasterizer.cullMode = VK_CULL_MODE_NONE;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional

		// Color blending combines color from fragment shader with color already in framebuffer
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE; // If set to 'VK_FALSE', final color is set to the original color from the fragemnt shader
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f; // Optional
		colorBlending.blendConstants[1] = 0.0f; // Optional
		colorBlending.blendConstants[2] = 0.0f; // Optional
		colorBlending.blendConstants[3] = 0.0f; // Optional

		// States that can technically be altered without re-creating the entire graphics pipeline 
		VkDynamicState dynamicStates[] =
		{
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_LINE_WIDTH
		};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = 2;
		dynamicState.pDynamicStates = dynamicStates;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1; // Optional
		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout; // Optional
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

		if (vkCreatePipelineLayout(vkSettings->logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create pipeline layout!");
		}
		LOG("Pipeline layout created.");

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;

		// Reference all the structures defining the fixed-function state
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = nullptr;

		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0; // Index of subpass to use

		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional - used if creating a derived pipeline with similar attributes of this one
		pipelineInfo.basePipelineIndex = -1;

		if (vkCreateGraphicsPipelines(vkSettings->logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create graphics pipeline!");
		}
		LOG("Graphics pipeline created.");

		vkDestroyShaderModule(vkSettings->logicalDevice, fragShaderModule, nullptr);
		vkDestroyShaderModule(vkSettings->logicalDevice, vertShaderModule, nullptr);
	}

	/*
	* Determine the resolution of the window in pixels and bind between the min / max image width / height supported by the capabilities of the physical device
	*/
	VkExtent2D Renderer::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
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

	VkSurfaceFormatKHR Renderer::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
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

		throw std::runtime_error("No valid swap surface format found!");
	}

	VkPresentModeKHR Renderer::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
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
	* Create framebuffers to reference all of the imageViews that represent the attachments
	* Each framebuffer associates each ImageView (and its image) with the renderPass / subpass / attachments we want to run on each
	*/
	void Renderer::createFramebuffers()
	{
		// Create a single framebuffer for each imageView in the swapchain
		swapchainFramebuffers.resize(swapchainImageViews.size());

		for (size_t i = 0; i < swapchainImageViews.size(); i++)
		{
			VkImageView attachments[] =
			{
				swapchainImageViews[i]
			};

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass; // Which renderPass the framebuffer needs to be compatible with
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments; // Specifies the imageViews that should be bound to the attachment descriptions in the renderPass 'pAttachment' array
			framebufferInfo.width = swapchainExtent.width;
			framebufferInfo.height = swapchainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(vkSettings->logicalDevice, &framebufferInfo, nullptr, &swapchainFramebuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create framebuffer!");
			}
		}
		LOG("Framebuffers created.");
	}

	void Renderer::createCommandPool()
	{
		// Command pools manage the memory that's used to store the command buffers
		VulkanSettings::QueueFamilyIndices queueFamilyIndices = vkSettings->findQueueFamilies(vkSettings->physicalDevice);

		VkCommandPoolCreateInfo commandPoolInfo{};
		commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
		commandPoolInfo.flags = 0; // Optional

		// Command buffers are executed by submitting them on one of the device queues (graphics, presentation, etc.)
		if (vkCreateCommandPool(vkSettings->logicalDevice, &commandPoolInfo, nullptr, &commandPool) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create the command pool!");
		}
		LOG("Command pool created.");
	}

	void Renderer::createCommandBuffers()
	{
		// One of the drawing commands involves binding the correct framebuffer 
		// so a new command buffer needs to be created for each image in the swapchain
		commandBuffers.resize(swapchainFramebuffers.size());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

		if (vkAllocateCommandBuffers(vkSettings->logicalDevice, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate command buffers!");
		}
		LOG("Command buffers allocated.");

		// Command buffer recording
		for (size_t i = 0; i < commandBuffers.size(); i++)
		{
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = 0; // Optional - specifies how we're going to use the command buffer i.e. one-time submit, per renderPass, etc.
			beginInfo.pInheritanceInfo = nullptr; // Optional - only relevant for secondary command buffers (specifies which state to inherit from calling primary command buffer)

			if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to begin recording command buffer!");
			}
			// Begin the RenderPass
			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = renderPass;
			renderPassInfo.framebuffer = swapchainFramebuffers[i];

			// Define the size of the render area
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = swapchainExtent;

			VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;

			// Parameters: command buffer to record command to, details of the RenderPass provided, how the drawing commands within the RenderPass will be provided
			// VK_SUBPASS_CONTENTS_INLINE - RenderPass commands embedded directly in primary command buffer rather than secondary buffer
			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			// Bind the graphics pipeline to the command buffer
			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

			// Bind the vertex buffer during rendering operations
			VkBuffer vertexBuffers[] = { vertexBuffer.getBuffer()};
			VkDeviceSize offsets[] = { 0 };

			// First two parameters (besides command buffer) specifies the offset and number of bindings to specify vertex buffers for
			vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);

			vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);

			// Bind a descriptor set to the current command buffer and the graphics pipeline (not compute pipeline)
			// 'PipelineLayout' is the layotut the descriptors are based on
			vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[i], 0, nullptr);

			// Parameters: the command buffer to bind to, vertex count, instance count (always 1 except for 'instance rendering'), offset for the first vertex (lowest value of the gl_VertexIndex), first instance (offset of instance rendering - gl_InstanceIndex)
			vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

			vkCmdEndRenderPass(commandBuffers[i]);

			if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to record command buffer!");
			}
		}
		LOG("Command buffers recorded.");
	}

	/*
	* Semaphores are needed to ensure that the command queues
	* (acquiring image from swapchain, execute command buffer with that image, return image to swapchain for presentation) don't occur out of order
	* No more than two frames are actively being worked on while the third is presenting and none of these frames are accidentally using the same image
	*/
	void Renderer::createSyncObjects()
	{
		// Allow frame that the graphics pipeline can utilize to have its own semaphores to the pipeline
		// can begin working on creating the next one while the current is still rendering
		imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		// Track if a frame in flight is currently using a swapchain image
		// Initially, no frame is currently using the image so initialize it to 'no fence'
		imagesInFlight.resize(swapchainImages.size(), VK_NULL_HANDLE);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

		// This 'tricks' the CPU into thinking a fence has just finished 
		// this is necessary as all are created in 'unsignaled' state by default which would cause 'vkWaitForFences' in 'drawFrame' to wait forever
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			if (vkCreateSemaphore(vkSettings->logicalDevice, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS
				|| vkCreateSemaphore(vkSettings->logicalDevice, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS
				|| vkCreateFence(vkSettings->logicalDevice, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create semaphores for a frame!");
			}
		}
		LOG("Semaphores created.");
	}

	void Renderer::recreateSwapchain()
	{
		// If the window is minimized (resized to zero), wait for the framebuffer to be in the foreground again
		int width = 0;
		int height = 0;
		glfwGetFramebufferSize(&Window::getInstance(), &width, &height);
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(&Window::getInstance(), &width, &height);
			glfwWaitEvents();
		}

		// Wait so we don't touch resources that may currently be in use
		vkDeviceWaitIdle(vkSettings->logicalDevice);

		cleanUpSwapchain();

		// If the window is resized, we need to recreate the swapchain, imageViews, and all resources that refer to the render space
		createSwapChain();
		createImageViews();
		createRenderPass();
		createGraphicsPipeline();
		createFramebuffers();
		createUniformBuffers();
		createDescriptorPool();
		createDescriptorSets();
		createCommandBuffers();
	}

	/*
	* Clean up all resources associated to objects used in the swapchain
	*/
	void Renderer::cleanUpSwapchain()
	{
		for (auto framebuffer : swapchainFramebuffers)
		{
			vkDestroyFramebuffer(vkSettings->logicalDevice, framebuffer, nullptr);
		}
		LOG("Framebuffers destroyed.");

		// Free up the existing command buffers rather than re-allocating them again in the command pool (wasteful)
		vkFreeCommandBuffers(vkSettings->logicalDevice, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

		vkDestroyPipeline(vkSettings->logicalDevice, graphicsPipeline, nullptr);
		LOG("Graphics pipeline destroyed.");

		vkDestroyPipelineLayout(vkSettings->logicalDevice, pipelineLayout, nullptr);
		LOG("Pipeline layout destroyed.");

		vkDestroyRenderPass(vkSettings->logicalDevice, renderPass, nullptr);
		LOG("RenderPass destroyed.");

		for (auto imageView : swapchainImageViews)
		{
			vkDestroyImageView(vkSettings->logicalDevice, imageView, nullptr);
		}
		LOG("Image views destroyed.");

		vkDestroySwapchainKHR(vkSettings->logicalDevice, swapchain, nullptr);
		LOG("Swapchain destroyed.");

		for (size_t i = 0; i < swapchainImages.size(); i++)
		{
			vkDestroyBuffer(vkSettings->logicalDevice, uniformBuffers.at(i).getBuffer(), nullptr);
			vkFreeMemory(vkSettings->logicalDevice, uniformBuffers.at(i).getBufferMemory(), nullptr);
		}

		vkDestroyDescriptorPool(vkSettings->logicalDevice, descriptorPool, nullptr);
	}

	void Renderer::framebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		auto app = reinterpret_cast<Renderer*>(glfwGetWindowUserPointer(window));
		app->framebufferResized = true;
	}

	void Renderer::createDescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

		// Specify in which shader stage the descriptor will be referenced
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = &uboLayoutBinding;

		if (vkCreateDescriptorSetLayout(vkSettings->logicalDevice, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create descriptor set layout!");
		}
		LOG("Descriptor set layout created.");
	}

	void Renderer::createUniformBuffers()
	{
		VkDeviceSize bufferSize = sizeof(UniformBuffer::UniformBufferObject);

		uniformBuffers.resize(swapchainImages.size());

		for (size_t i = 0; i < swapchainImages.size(); i++)
		{
			Buffer::createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers.at(i).getBuffer(), uniformBuffers.at(i).getBufferMemory());
		}
	}

	void Renderer::createVertexBuffers()
	{
		vertexBuffer = VertexBuffer(vertices, graphicsQueue, commandPool);
	}

	void Renderer::createIndexBuffers()
	{
		indexBuffer = IndexBuffer(indices, graphicsQueue, commandPool);
	}

	void Renderer::createDescriptorPool()
	{
		// Describe which descriptor types the descriptor sets will contain
		VkDescriptorPoolSize poolSize{};
		poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSize.descriptorCount = static_cast<uint32_t>(swapchainImages.size());

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &poolSize;

		// Specify the maximum number of descriptor sets that may be allocated
		poolInfo.maxSets = static_cast<uint32_t>(swapchainImages.size());
	
		poolInfo.flags = 0; // Optional

		if (vkCreateDescriptorPool(vkSettings->logicalDevice, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create descriptor pool!");
		}
		LOG("Descriptor pool created.");
	}

	void Renderer::createDescriptorSets()
	{
		// Create one descriptor set for each swapchain image, all with the same layout
		std::vector<VkDescriptorSetLayout> layouts(swapchainImages.size(), descriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(swapchainImages.size());
		allocInfo.pSetLayouts = layouts.data();

		// Allocate descriptor sets each with one uniform buffer descriptor
		descriptorSets.resize(swapchainImages.size());
		if (vkAllocateDescriptorSets(vkSettings->logicalDevice, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate descriptor sets!");
		}
		LOG("Descriptor sets allocated.");

		// Populate each uniform buffer descriptor with a uniform buffer
		for (size_t i = 0; i < swapchainImages.size(); i++)
		{
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = uniformBuffers.at(i).getBuffer();
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBuffer::UniformBufferObject);

			// Update the configuration of the descriptor sets
			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;

			// Descriptor set to update
			descriptorWrite.dstSet = descriptorSets[i];

			// Binding of the descriptor set
			descriptorWrite.dstBinding = 0;

			// Descriptors can be arrays so specify the first index in the array that is to be updated
			descriptorWrite.dstArrayElement = 0;

			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

			// Specify the number of array elements to be updated (possible to update multiple at once)
			descriptorWrite.descriptorCount = 1;

			descriptorWrite.pBufferInfo = &bufferInfo;
			descriptorWrite.pImageInfo = nullptr; // Optional
			descriptorWrite.pTexelBufferView = nullptr; // Optional

			vkUpdateDescriptorSets(vkSettings->logicalDevice, 1, &descriptorWrite, 0, nullptr);
		}
	}

	void Renderer::drawFrame()
	{
		// Wait for the fence to ensure the current frame is finished being presented
		// Waits for any of the fences to be finished before returning - VK_TRUE indicates to wait for all fences
		vkWaitForFences(vkSettings->logicalDevice, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

		// Acquire an image from the swapchain
		// UINT64_MAX disables timeout in nanoseconds for an image to become available
		// Image index refers to the image in swapchain image array. This is used to select the correct command buffer
		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(vkSettings->logicalDevice, swapchain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

		// Detect a window resize, recreate the swapchain to match the new resolution, and try presenting on the next 'drawFrame'
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			recreateSwapchain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("Failed to acquire swapchain image!");
		}

		uniformBuffers.at(imageIndex).update(swapchainExtent);

		// Check if a previous frame is using this image (i.e. there is its fence to wait on)
		if (imagesInFlight[imageIndex] != VK_NULL_HANDLE)
		{
			vkWaitForFences(vkSettings->logicalDevice, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
		}

		// Mark the image as currently being 'in use' by this frame
		imagesInFlight[imageIndex] = inFlightFences[currentFrame];

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		// Specify which semaphores to wait on before execution begins and in which stage of the pipeline to wait
		// In this case, don't write colors to the image until it's available
		VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		// Specify which command buffer to submit for execution (ensure the command buffer used (imageIndex) corresponds to the image in the swapchain associated to the command buffer
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

		// Specify which semaphores to signal once the previous command buffer has finished execution (let the 'renderFinishedSemaphore' know it's good to go)
		VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(vkSettings->logicalDevice, 1, &inFlightFences[currentFrame]);

		// Submit the command buffer to the graphics queue
		// Utilize the current fence to synchronize the CPU operations with the rendering of frames on the GPU (prevents the GPU from being overwhelmed)
		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to submit the draw command buffer!");
		}

		// Submit the result to the swapchain for presentation
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		// Which semaphores to wait on before presentation can happen
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapchains[] = { swapchain };
		presentInfo.swapchainCount = 1;

		// Swapchain to present images to and the index of the image for each swapchain
		presentInfo.pSwapchains = swapchains;
		presentInfo.pImageIndices = &imageIndex;

		presentInfo.pResults = nullptr;

		// Present an image to the swapchain
		result = vkQueuePresentKHR(presentQueue, &presentInfo);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized)
		{
			framebufferResized = false;
			recreateSwapchain();
		}
		else if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to present swapchain image!");
		}

		// Advance to the next frame
		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void Renderer::calculateFPS()
	{
		static std::chrono::time_point<std::chrono::steady_clock> oldTime = std::chrono::high_resolution_clock::now();
		static uint32_t fps;

		fps++;

		if (std::chrono::duration_cast<std::chrono::seconds>(
			std::chrono::high_resolution_clock::now() - oldTime) >= std::chrono::seconds{ 1 }) {
			oldTime = std::chrono::high_resolution_clock::now();
			std::stringstream ss;
			ss << Window::getWindowTitle() << " | FPS: " << fps;
			glfwSetWindowTitle(&Window::getInstance(), ss.str().c_str());
			fps = 0;
		}
	}
}
