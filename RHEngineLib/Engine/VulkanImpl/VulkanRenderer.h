#pragma once
#include "../IRenderer.h"
#include "../../stdafx.h"
namespace RHEngine
{
	class VulkanRenderer : public IRenderer
	{
	public:

		VulkanRenderer(HWND window, HINSTANCE inst);
		~VulkanRenderer();

		virtual bool InitDevice() override;

		virtual bool ShutdownDevice() override;

		virtual bool GetAdaptersCount(int &) override;

		virtual bool GetAdapterInfo(unsigned int n, std::wstring &) override;

		virtual bool SetCurrentAdapter(unsigned int n) override;

		virtual bool GetOutputCount(unsigned int adapterId, int &) override;

		virtual bool GetOutputInfo(unsigned int n, std::wstring &) override;

		virtual bool SetCurrentOutput(unsigned int id) override;

		virtual bool GetDisplayModeCount(unsigned int outputId, int &) override;

		virtual bool GetDisplayModeInfo(unsigned int id, DisplayModeInfo &) override;

		virtual bool SetCurrentDisplayMode(unsigned int id) override;

		virtual bool GetCurrentAdapter(int &) override;

		virtual bool GetCurrentOutput(int &) override;

		virtual bool GetCurrentDisplayMode(int &) override;
		virtual bool Present(void* image) override;
	private:
		UINT m_uiCurrentAdapter = 0,
			m_uiCurrentOutput = 0,
			m_uiCurrentAdapterMode = 0;
		// Vulkan renderer instance - used to work with platform-specific stuff
		VkInstance m_vkInstance;
		// Avaliable physical devices
		std::vector<VkPhysicalDevice> m_aAdapters;

		// Main window surface 
		VkSurfaceKHR m_vkWindowSurface = VK_NULL_HANDLE;
		// Main window surface capabilities
        VkSurfaceCapabilitiesKHR m_vkWindowSurfaceCap = {};
		// Avaliable surface formats for main window
		std::vector<VkSurfaceFormatKHR> m_aWindowSurfaceFormats;
		// Avaliable present modes for main window
		std::vector<VkPresentModeKHR> m_aWindowSurfacePresentModes;

		// Main logical device 
		VkDevice m_vkDevice = VK_NULL_HANDLE;
		// Vulkan debug callback
		VkDebugReportCallbackEXT m_debugCallback;
		// Graphics families list
		std::vector<int> m_aGraphicsQueueFamilies;
		// Compute families list
		std::vector<int> m_aComputeQueueFamilies;
		// Transfer families list
		std::vector<int> m_aTransferQueueFamilies;
		// Graphics queue - used as immediate context
		VkQueue m_vkGraphicsQueue = VK_NULL_HANDLE;
		// Present queue
		VkQueue m_vkPresentQueue = VK_NULL_HANDLE;
		// Swap-chain image retrieval semaphore
		VkSemaphore m_vkSwapChainImageSemaphore = VK_NULL_HANDLE;
		VkSemaphore m_vkMainCmdBufferSemaphore = VK_NULL_HANDLE;
        VkFence m_vkMainCmdBufferFence = VK_NULL_HANDLE;
        std::vector<VkFence> m_vkMainCmdBufferFences;
		// Main command pool for graphics queue
		VkCommandPool m_vkGraphicsQueueCommandPool = VK_NULL_HANDLE;
		// Main command buffer
		std::vector<VkCommandBuffer> m_vkMainCommandBufferList;
		uint32_t m_nCurrentMainCmdBuffer;
		// Unused
		std::vector<VkDisplayPropertiesKHR> m_aOutputProperties;
		// Instance extension list
		std::vector<const char*> m_aExtensions;
		// Instance layer list
		std::vector<const char*> m_aLayers;
		// Main swap-chain
		VkSwapchainKHR m_vkSwapChain = VK_NULL_HANDLE;

		// Унаследовано через IRenderer
		virtual void * AllocateImageBuffer(unsigned int width, unsigned int height, RHImageBufferType type) override;
		virtual bool FreeImageBuffer(void * buffer, RHImageBufferType type) override;

		// Унаследовано через IRenderer
		virtual bool BindImageBuffers(RHImageBindType bindType, const std::unordered_map<int, void*>& buffers) override;

		// Унаследовано через IRenderer
		virtual bool ClearImageBuffer(RHImageClearType clearType, void* buffer, const float clearColor[4]) override;

		// Унаследовано через IRenderer
		virtual bool BeginCommandList(void * cmdList) override;
		virtual bool EndCommandList(void * cmdList) override;

		// Унаследовано через IRenderer
		virtual bool RequestSwapChainImage(void * frameBuffer) override;
		virtual bool PresentSwapChainImage(void * frameBuffer) override;
	};
};