#include "stdafx.h"
#include "VulkanShader.h"
#include "CDebug.h"

static std::vector<char> readFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		g_pDebug->printError("failed to open file!");
	}
	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	return buffer;
}
CVulkanShader::CVulkanShader(CVulkanDevice *vkDev, std::string filePath, char* entryName, VkShaderStageFlagBits type)
{
	m_pVkDevice = vkDev;
	auto shaderCode = readFile(filePath);
	VKCreateShaderModule(shaderCode, m_vkShaderModule);
	m_vkShaderStageInfo = {};
	m_vkShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	m_vkShaderStageInfo.stage = type;
	m_vkShaderStageInfo.module = m_vkShaderModule;
	m_vkShaderStageInfo.pName = entryName;

}
void CVulkanShader::VKCreateShaderModule(const std::vector<char>& code, VkShaderModule& shaderModule) {
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = (uint32_t*)code.data();
	if (vkCreateShaderModule(m_pVkDevice->getDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		g_pDebug->printError("failed to create shader module!");
	}
}

CVulkanShader::~CVulkanShader()
{
	vkDestroyShaderModule(m_pVkDevice->getDevice(), m_vkShaderModule, nullptr);
}
