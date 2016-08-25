#ifndef VulkanRenderer_h__
#define VulkanRenderer_h__
#include "VulkanSwapChain.h"
class CVulkanDevice;
class CVulkanCommandBufferMgr;
class CVulkanRenderer
{
public:
	CVulkanRenderer(HWND& window);

	~CVulkanRenderer();

	// Vulkan Device stuff
	void InitDevice();

	void m_initRenderPass();

	void DeInitDevice();

	void BeginScene();
	void ClearScene(VkClearColorValue& clear_color);
	void EndScene();

	void Present();
	// Renderer get stuff
	VkInstance&						getInstance()			{ return m_Instance; }
	CVulkanDevice*					getDevice()				{ return m_pDevice; }
	VkFramebuffer&					getCurrentFrameBuffer()	{ return m_pSwapChain->getFrameBufferList()[m_currentBuffer]; }
	VkRenderPass&					getRenderPass()			{ return m_renderPass; }
	CVulkanCommandBufferMgr*		getBufferMgr()			{ return m_pCommandBufferMgr; }
	std::vector<VkPhysicalDevice>	getGPUlist()			{ return m_GPU_list; }
	VkPhysicalDevice				getCurrentGPU()			{ return m_GPU_list[m_GPU_ID]; }
	uint32_t						getWindowWidth()		{ return m_windowWidth; }
	uint32_t						getWindowHeight()		{ return m_windowHeight; }
	VkSurfaceKHR					getWindowSurface()		{ return m_windowSurface; }
	VkPhysicalDeviceProperties		getGPUProperties(int devID) {
		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(m_GPU_list[devID], &props);
		return props;
	}
	VkPhysicalDeviceProperties		getGPUProperties() const { return m_GPU_Properties; }

	int		GPU_ID() const { return m_GPU_ID; }
	void	GPU_ID(int val) { m_GPU_ID = val; }
	
private:
	// Vulkan Instance stuff
	void m_initInstance();
	void m_deInitInstance();

	void m_initDebugCB();
	void m_deInitDebugCB();

	void m_initWindowSurface(HWND& window);
	void m_deInitWindowSurface();

	void m_initSemaphores();
	void m_deInitSemaphores();
private:
	CVulkanDevice*					m_pDevice				= nullptr;
	CVulkanCommandBufferMgr*		m_pCommandBufferMgr		= nullptr;
	CVulkanSwapChain*				m_pSwapChain			= nullptr;
	VkInstance						m_Instance				= VK_NULL_HANDLE;
	VkDebugReportCallbackEXT		m_debugCB				= VK_NULL_HANDLE;
	VkSurfaceKHR					m_windowSurface			= VK_NULL_HANDLE;
	VkSemaphore						m_imageAquiredSemaphore = VK_NULL_HANDLE;
	VkSemaphore						m_renderFinishSemaphore = VK_NULL_HANDLE;
	VkRenderPass					m_renderPass			= VK_NULL_HANDLE;
	VkFence							m_renderFence;

	std::vector<VkPhysicalDevice>	m_GPU_list			= { };
	VkPhysicalDeviceProperties		m_GPU_Properties	= { };
	uint32_t						m_GPU_ID			= 0;

	std::vector<const char *>		m_layers			= { };
	std::vector<const char *>		m_extensions		= { };

	uint32_t						m_windowWidth			= 640;
	uint32_t						m_windowHeight			= 480;
	uint32_t						m_currentBuffer			= 0;
};
void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator);
VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback);
VKAPI_ATTR VkBool32 VKAPI_CALL RwVKDebugCB(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location,
	int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData);
#endif // VulkanRenderer_h__

