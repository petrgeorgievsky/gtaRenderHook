#ifndef VulkanShader_h__
#define VulkanShader_h__
#include "VulkanDevice.h"
class CVulkanShader
{
public:
	CVulkanShader(CVulkanDevice *vkDev,std::string filePath, char* entryName,VkShaderStageFlagBits type);
	~CVulkanShader();
	void VKCreateShaderModule(const std::vector<char>& code, VkShaderModule& shaderModule);
	VkPipelineShaderStageCreateInfo& getStageCreateInfo() { return m_vkShaderStageInfo; }
private:
	VkShaderModule m_vkShaderModule;
	VkPipelineShaderStageCreateInfo m_vkShaderStageInfo;
	CVulkanDevice* m_pVkDevice;
};
#endif // VulkanShader_h__

