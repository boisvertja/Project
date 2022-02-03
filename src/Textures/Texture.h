#pragma once
#include "../Renderer/Buffer.h"
#include <stb_image.h>

namespace VulkanProject
{
	class Texture
	{
	public:
		Texture(const char* imagePath, VkQueue& graphicsQueue, VkCommandPool& commandPool);
		~Texture();

		int getWidth() const;
		int getHeight() const;
		int getChannels() const;

		VkImage& getTextureImage();
		VkDeviceMemory& getTextureImageMemory();

	private:
		void createImage(VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
		void mapPixelMemory(stbi_uc* pixels, VkDeviceSize imageSize, Buffer stagingBuffer);
		void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	private:
		int width;
		int height;
		int channels;

		VkImage textureImage;
		VkDeviceMemory textureImageMemory;

		VkQueue graphicsQueue;
		VkCommandPool commandPool;
	};
}
