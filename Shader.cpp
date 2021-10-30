#include "Shader.h"

Shader::Shader(Shader::SHADER_TYPE shaderType, VkDevice logicalDevice)
{
	this->shaderType = shaderType;
	this->logicalDevice = logicalDevice;
	
	readShaderFile();
	createShaderModule();
}

Shader::~Shader()
{
}

Shader::SHADER_TYPE Shader::getShaderType() const
{
	return shaderType;
}

std::string Shader::getPath() const
{
	if (shaderType == SHADER_TYPE::VERTEX)
	{
		return std::string("Shaders/vert.spv");
	}
	else if (shaderType == SHADER_TYPE::TESSELLATION)
	{
		return std::string("Shaders/tess.spv");
	}
	else if (shaderType == SHADER_TYPE::GEOMETRY)
	{
		return std::string("Shaders/geo.spv");
	}
	else if (shaderType == SHADER_TYPE::FRAGMENT)
	{
		return std::string("Shaders/frag.spv");
	}

	throw std::runtime_error("No valid shader type found!");
}

std::vector<char> Shader::getShaderCode() const
{
	return code;
}

VkShaderModule Shader::getShaderModule() const
{
	return shaderModule;
}

void Shader::createShaderModule()
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();

	// Cast pointer from char* to uint32_t* using reinterpret cast
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	if (vkCreateShaderModule(logicalDevice, &createInfo, nullptr, &shaderModule) != VkResult::VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create shader module!");
	}
}

void Shader::readShaderFile()
{
	std::string filename = getPath();

	// 'ate' specifies reading at the end of the file
	// binary states the file should be read as a binary file
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error("Failed to read shader file!");
	}

	// Use 'ate' to begin at the end of the file in order to allocate the buffer to be the size of the file's contents
	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	// Return to the beginning of the file and read all the bytes at once
	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();
	code = buffer;
}
