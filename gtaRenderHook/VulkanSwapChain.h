#ifndef VulkanSwapChain_h__
#define VulkanSwapChain_h__
class CVulkanRenderer;

class CVulkanSwapChain
{
public:
	CVulkanSwapChain(CVulkanRenderer* pRenderer);
	~CVulkanSwapChain();

	void CreateFrameBuffers();
	std::vector<VkImage>		&getImageList()			{ return m_swapChainImages; }
	std::vector<VkFramebuffer>	&getFrameBufferList()	{ return m_frameBuffers; }
	VkSurfaceFormatKHR			&getSurfaceFormat()		{ return m_surfaceFormat; }
	VkSwapchainKHR				&getSwapChain()			{ return m_swapChain; }
private:
	VkPresentModeKHR	m_findPresentMode(VkPhysicalDevice gpu, VkSurfaceKHR surface);
	VkSurfaceFormatKHR	m_findSurfaceFormat();
private:
	CVulkanRenderer*				m_pRenderer				= nullptr;
	VkSwapchainKHR					m_swapChain				= VK_NULL_HANDLE;
	uint32_t						m_swapChainImageCount	= 2;
	VkSurfaceFormatKHR				m_surfaceFormat;
	VkSurfaceCapabilitiesKHR		m_surfaceCaps;

	std::vector<VkImage>			m_swapChainImages		= {};
	std::vector<VkImageView>		m_swapChainImageViews	= {};
	std::vector<VkFramebuffer>		m_frameBuffers			= {};
};
#endif // VulkanSwapChain_h__
