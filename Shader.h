#pragma once
#include "stdafx.h"
#include <fstream>

class Shader
{
public:

	enum class SHADER_TYPE
	{
		VERTEX,
		TESSELLATION,
		GEOMETRY,
		FRAGMENT
	};

	Shader(Shader::SHADER_TYPE shaderType, VkDevice logicalDevice);
	~Shader();
	SHADER_TYPE getShaderType() const;
	VkShaderModule getShaderModule() const;

private:
	void readShaderFile();
	void createShaderModule();
	std::string getPath() const;
	std::vector<char> getShaderCode() const;

private:
	Shader::SHADER_TYPE shaderType;
	VkShaderModule shaderModule;
	VkDevice logicalDevice;
	std::vector<char> code;
};
