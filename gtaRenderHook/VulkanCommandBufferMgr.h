#ifndef VulkanCommandBufferMgr_h__
#define VulkanCommandBufferMgr_h__

class CVulkanDevice;

class CVulkanCommandBufferMgr
{
public:
	CVulkanCommandBufferMgr(CVulkanDevice* pDevice);
	~CVulkanCommandBufferMgr();
	
	void BeginRenderBuffer(VkImage& image);
	void EndRenderBuffer(VkImage& image);

	VkCommandBuffer& getRenderCommandBuffer() { return m_renderBuffer; }

private:
	VkCommandPool	m_pool			= VK_NULL_HANDLE;
	VkCommandBuffer m_renderBuffer	= VK_NULL_HANDLE;
	std::vector<VkCommandBuffer> m_renderBufferList{};
	CVulkanDevice*	m_pDevice		= nullptr;
	int m_firstTimeInit = -2;
};
#endif // VulkanCommandBufferMgr_h__

