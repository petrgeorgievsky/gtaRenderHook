#ifndef VulkanPipeline_h__
#define VulkanPipeline_h__
class CVulkanShader;
class CVulkanDevice;
class CVulkanPipeline
{
public:
	CVulkanPipeline(CVulkanDevice* pDevice, VkRenderPass &renderPass, std::string name,
		std::vector<VkVertexInputBindingDescription> inputBindigDesc, std::vector<VkVertexInputAttributeDescription> vertexAtrributeBinding, VkPrimitiveTopology topology, VkDescriptorSetLayout* descSets);
	~CVulkanPipeline();
	VkPipeline			&getPipeline() { return m_pipeline; }
	VkPipelineLayout	&getPipelineLayout() { return m_pipeline_layout; }
private:
	CVulkanShader*		m_pVS;
	CVulkanShader*		m_pPS;
	CVulkanDevice*		m_pDevice;
	VkPipelineLayout	m_pipeline_layout;
	VkPipeline			m_pipeline;
};
#endif // VulkanPipeline_h__

