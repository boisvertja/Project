#pragma once
#include "../Textures/Texture.h"

namespace VulkanProject
{
	class Scene
	{
	public:
		bool isEmpty() const;
		void addTextureToScene(Texture& texture);
		void removeTextureFromScene(Texture& texture);
		std::vector<Texture> getTexturesInScene() const;

	private:
		std::vector<Texture> textures = std::vector<Texture>();
	};
}
