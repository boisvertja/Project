#include "Scene.h"

namespace VulkanProject
{
	bool Scene::isEmpty() const
	{
		return textures.empty();
	}

	void Scene::addTextureToScene(Texture& texture)
	{
		textures.push_back(texture);
	}

	std::vector<Texture> Scene::getTexturesInScene() const
	{
		return textures;
	}
}
