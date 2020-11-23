#pragma once
#include "../Common/IStateCacheObject.h"
#include "../IRenderer.h"
#include <common.h>

namespace rh::engine {
class VulkanRenderer : public IRenderer
{
public:
    VulkanRenderer( HWND window, HINSTANCE inst );
    ~VulkanRenderer() override;

    bool InitDevice() override;

    bool ShutdownDevice() override;

    bool GetAdaptersCount( unsigned int & ) override;

    bool GetAdapterInfo( unsigned int n, rh::engine::String & ) override;

    bool SetCurrentAdapter( unsigned int n ) override;

    bool GetOutputCount( unsigned int adapterId, int & ) override;

    bool GetOutputInfo( unsigned int n, std::wstring & ) override;

    bool SetCurrentOutput( unsigned int id ) override;

    bool GetDisplayModeCount( unsigned int outputId, int & ) override;

    bool GetDisplayModeInfo( unsigned int id, DisplayModeInfo & ) override;

    bool SetCurrentDisplayMode( unsigned int id ) override;

    bool GetCurrentAdapter( int & ) override;

    bool GetCurrentOutput( int & ) override;

    bool GetCurrentDisplayMode( int & ) override;

    bool Present( void *image ) override;

    void *GetCurrentDevice() override;
    void *GetCurrentContext() override;

    void BindViewPorts( const std::vector<ViewPort> &viewports ) override;

private:
    UINT m_uiCurrentAdapter = 0, m_uiCurrentOutput = 0, m_uiCurrentAdapterMode = 0;

    // Vulkan renderer instance - used to work with platform-specific stuff
    vk::Instance m_vkInstance;

    // Avaliable physical devices
    std::vector<vk::PhysicalDevice> m_aAdapters;

    // Main window surface
    vk::SurfaceKHR m_vkWindowSurface = nullptr;

    // Main window surface capabilities
    vk::SurfaceCapabilitiesKHR m_vkWindowSurfaceCap = {};

    // Avaliable surface formats for main window
    std::vector<vk::SurfaceFormatKHR> m_aWindowSurfaceFormats;

    // Avaliable present modes for main window
    std::vector<vk::PresentModeKHR> m_aWindowSurfacePresentModes;

    // Main logical device
    vk::Device m_vkDevice = nullptr;

    // Vulkan debug callback
    VkDebugReportCallbackEXT m_debugCallback{};

    // Graphics families list
    std::vector<uint32_t> m_aGraphicsQueueFamilies;

    // Compute families list
    std::vector<uint32_t> m_aComputeQueueFamilies;

    // Transfer families list
    std::vector<uint32_t> m_aTransferQueueFamilies;

    // Graphics queue - used as immediate context
    vk::Queue m_vkGraphicsQueue = nullptr;

    // Present queue
    vk::Queue m_vkPresentQueue = nullptr;

    // Swap-chain image retrieval semaphore
    vk::Semaphore m_vkSwapChainImageSemaphore = nullptr;

    vk::Semaphore m_vkMainCmdBufferSemaphore = nullptr;
    vk::Fence m_vkMainCmdBufferFence = nullptr;
    std::vector<vk::Fence> m_vkMainCmdBufferFences;

    // Main command pool for graphics queue
    vk::CommandPool m_vkGraphicsQueueCommandPool = nullptr;

    // Main command buffer
    std::vector<vk::CommandBuffer> m_vkMainCommandBufferList;
    uint32_t m_nCurrentMainCmdBuffer = 0;

    // Unused
    std::vector<vk::DisplayPropertiesKHR> m_aOutputProperties;

    // Instance extension list
    std::vector<const char *> m_aExtensions;

    // Instance layer list
    std::vector<const char *> m_aLayers;

    // Main swap-chain
    vk::SwapchainKHR m_vkSwapChain = nullptr;

    void *AllocateImageBuffer( const ImageBufferInfo &info ) override;
    bool FreeImageBuffer( void *buffer, ImageBufferType type ) override;

    bool BindImageBuffers( ImageBindType bindType,
                           const std::vector<IndexPtrPair> &buffers ) override;

    bool ClearImageBuffer( ImageClearType clearType,
                           void *buffer,
                           const std::array<float, 4> &clearColor ) override;

    bool BeginCommandList( void *cmdList ) override;
    bool EndCommandList( void *cmdList ) override;

    bool RequestSwapChainImage( void *frameBuffer ) override;
    bool PresentSwapChainImage( void *frameBuffer ) override;
    void FlushCache() override;

    IGPUAllocator *GetGPUAllocator() override;

    void InitImGUI() override;
    void ImGUIStartFrame() override;

    void ImGUIRender() override;
    void ShutdownImGUI() override;
};
} // namespace rh::engine
