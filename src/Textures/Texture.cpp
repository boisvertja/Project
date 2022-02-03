#include "Texture.h"
 
namespace VulkanProject
{
	Texture::Texture(const char* imagePath, VkQueue& graphicsQueue, VkCommandPool& commandPool)
	{
		this->graphicsQueue = graphicsQueue;
		this->commandPool = commandPool;

		stbi_uc* pixels = stbi_load(imagePath, &width, &height, &channels, STBI_rgb_alpha);

		// Pixels are laid out row by row with 4 bytes per pixel
		VkDeviceSize imageSize = (uint64_t)(width * height * 4);

		if (!pixels)
		{
			throw std::runtime_error("Failed to load texture image!");
		}

		Buffer stagingBuffer = Buffer();
		Buffer::createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer.getBuffer(), stagingBuffer.getBufferMemory());

		mapPixelMemory(pixels, imageSize, stagingBuffer);

		stbi_image_free(pixels);

		createImage(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		// Copy the staging buffer to the texture image.
		transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		copyBufferToImage(stagingBuffer.getBuffer(), textureImage, static_cast<uint32_t>(width), static_cast<uint32_t>(height));

		// Transition the texture image again to prepare it for shader access.
		transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		VkDevice logicalDevice = VulkanSettings::getInstance()->getLogicalDevice();

		vkDestroyBuffer(logicalDevice, stagingBuffer.getBuffer(), nullptr);
		vkFreeMemory(logicalDevice, stagingBuffer.getBufferMemory(), nullptr);
	}

	VulkanProject::Texture::~Texture()
	{
	}

	/*
	* Creates and binds image object according to the given image extent, usage, and memory requirements.
	*/
	void Texture::createImage(VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties)
	{
		VkDevice logicalDevice = VulkanSettings::getInstance()->getLogicalDevice();

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;

		// Image type tells Vulkan with what coordinate system the texels (pixels in image object) should be addressed
		imageInfo.imageType = VK_IMAGE_TYPE_2D;

		// Extent fields specify the dimensions of the image.
		imageInfo.extent.width = static_cast<uint32_t>(width);
		imageInfo.extent.height = static_cast<uint32_t>(height);
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;

		// Use the same image format for the texels as the pixels in the buffer
		imageInfo.format = format;

		// Either "linear" or "optimal". Linear lays out the pixels in row-major order.
		// Optimal lays out the pixels according to an implementation defined for optimal access
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		// The image will be used as the destination for the buffer to copy to so it should be setup as such.
		// We also need to be able to access the image from the shader to color the mesh, so the usage should include "sampled_bit".
		imageInfo.usage = usage;

		// The image will only be used by one queue family (the one that supports graphics and transfer operations).
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		// "Samples" is used for multisampling (not multisampling at the moment so only using '1').
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.flags = 0; // Optional

		if (vkCreateImage(logicalDevice, &imageInfo, nullptr, &textureImage) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create image!");
		}
		LOG("Image texture created.");

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(logicalDevice, textureImage, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = Buffer::findMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &textureImageMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate image memory!");
		}
		LOG("Image memory allocated.");

		vkBindImageMemory(logicalDevice, textureImage, textureImageMemory, 0);
	}

	void Texture::mapPixelMemory(stbi_uc* pixels, VkDeviceSize imageSize, Buffer stagingBuffer)
	{
		VkDevice logicalDevice = VulkanSettings::getInstance()->getLogicalDevice();

		void* data;
		vkMapMemory(logicalDevice, stagingBuffer.getBufferMemory(), 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(logicalDevice, stagingBuffer.getBufferMemory());
	}

	void Texture::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		VkCommandBuffer commandBuffer = Buffer::createCommandBuffer(commandPool);
		Buffer::beginCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		// Pipeline barrier used to synchronize access to resources (not reading while still writing)
		// Also used to transition image layouts and transfer queue family ownership
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;

		// If using the barrier to transfer queue family ownership, these two fields should be indices of the queue families
		// They should be set to 'VK_QUEUE_FAMILY_IGNORED' if you don't want to do this.
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		// Specify the image that is affected and the specific part of the image.
		// Mipmapping is not currently used so the levels and layers are set to 0 or 1
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = 0;

		// Specify the two transitions stages that should be used for synchronization.
		// Undefined -> Transfer destination: Transfer writes that don't need to wait on anything
		// Transfer destination -> Shader reading: Shader reads should wait on transfer writes, specifically the fragment shader
		// since that's where the texture will be used.
		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = 0; // Transfer writes don't need to wait on anything so can specify an empty access mask.
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT; // Pre-barrier operations
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else
		{
			throw std::invalid_argument("Unsupported image layout transition!");
		}

		vkCmdPipelineBarrier(
			commandBuffer,

			// Specify the pipeline stages that should occur before and after the barrier (synchronize)
			sourceStage, destinationStage,

			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);
		
		Buffer::endCommandBuffer(commandBuffer, graphicsQueue, commandPool);
	}
	
	/*
	* Copy pixel data from the buffer to the image. Specifies how the pixels are laid out in buffer memory
	* and which part of the image to copy pixels to.
	*/
	void Texture::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
	{
		VkCommandBuffer commandBuffer = Buffer::createCommandBuffer(commandPool);
		Buffer::beginCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		// Specify which part of the buffer will be copied to which part of the image.
		VkBufferImageCopy region{};

		// The byte offset in the buffer at which the pixel values start.
		region.bufferOffset = 0;

		// How the pixels are laid out in memory.
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		// To which part of the image we want to copy pixels.
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent =
		{
			width,
			height,
			1
		};

		// Copy the buffer to the image.
		vkCmdCopyBufferToImage(
			commandBuffer,
			buffer,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, // Specify the layout the image is currently using.
			1,
			&region
		);

		Buffer::endCommandBuffer(commandBuffer, graphicsQueue, commandPool);
	}

	int Texture::getWidth() const
	{
		return width;
	}

	int Texture::getHeight() const
	{
		return height;
	}

	int Texture::getChannels() const
	{
		return channels;
	}

	VkImage& Texture::getTextureImage()
	{
		return textureImage;
	}

	VkDeviceMemory& Texture::getTextureImageMemory()
	{
		return textureImageMemory;
	}
}
