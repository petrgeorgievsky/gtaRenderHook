#ifndef VulkanIm2DPipeline_h__
#define VulkanIm2DPipeline_h__

class CVulkanPipeline;
class CVulkanRenderer;

struct RenderWareVertex
{
	float x, y, z, w;
	float u, v;
};


class CVulkanIm2DPipeline
{
public:
	CVulkanIm2DPipeline(CVulkanRenderer* pRenderer);
	~CVulkanIm2DPipeline();

	void Draw(RwPrimitiveType prim,RwIm2DVertex* verticles,RwUInt32 vertexCount);
	void DrawIndexed(RwPrimitiveType prim,RwIm2DVertex* verticles, RwUInt32 vertexCount, RwImVertexIndex *indices, RwUInt32 numIndices);
	void EndScene();
	VkDescriptorSet		&getDescriptorSet() { return m_descriptorSet; }
	VkPipelineLayout	&getFanPipeLayout();
	VkPipelineLayout	&getListPipeLayout();
	void UpdateViewProj(RwMatrix& view, RwMatrix& proj);
private:
	uint32_t getMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties);
	CVulkanPipeline*		m_pBasePipeline_Fan;
	CVulkanPipeline*		m_pBasePipeline_List;
	CVulkanRenderer*		m_pRenderer;

	VkBuffer				m_dynamicVertexBuffer;
	VkDeviceMemory			m_dynamicVertexBufferMemory;
	VkBuffer				m_IndexBuffer;
	VkDeviceMemory			m_IndexBufferMemory;
	uint32_t				m_dynVB_offset=0;
	uint32_t				m_dynIB_offset=0;
	VkDescriptorSetLayout	m_descriptorSetLayout;
	VkDescriptorSet			m_descriptorSet;
	VkDescriptorPool		m_descriptorPool;

	struct {
		VkBuffer buffer;
		VkDeviceMemory memory;
		VkDescriptorBufferInfo descriptor;
	}  uniformDataVS;
	struct {
		RwMatrix projectionMatrix;
		RwMatrix viewMatrix;
	} uboVS;
};
#endif // VulkanIm2DPipeline_h__

