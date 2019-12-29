#include "VulkanRenderer.h"
#include "..\..\DebugUtils\DebugLogger.h"
#include "Engine/Common/types/image_buffer_info.h"
#include "Engine/Common/types/image_buffer_type.h"
#include "Engine/Common/types/image_clear_type.h"
#include "ImGUI/imgui.h"
#include "ImGUI/imgui_impl_vulkan.h"
#include "ImageBuffers\VulkanBackBuffer.h"
#include "VulkanCommon.h"

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback( VkDebugReportFlagsEXT /*flags*/,
                                                     VkDebugReportObjectTypeEXT /*objType*/,
                                                     uint64_t /*obj*/,
                                                     size_t /*location*/,
                                                     int32_t /*code*/,
                                                     const char * /*layerPrefix*/,
                                                     const char *msg,
                                                     void * /*userData*/ )
{
    rh::debug::DebugLogger::Log( ToRHString( msg ) );
    return VK_FALSE;
}

VkResult CreateDebugReportCallbackEXT( vk::Instance instance,
                                       const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
                                       VkDebugReportCallbackEXT *pCallback )
{
    auto func = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(
        instance.getProcAddr( "vkCreateDebugReportCallbackEXT" ) );

    if ( func != nullptr )
        return func( static_cast<VkInstance>( instance ), pCreateInfo, nullptr, pCallback );
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DestroyDebugReportCallbackEXT( vk::Instance instance,
                                    VkDebugReportCallbackEXT callback,
                                    const VkAllocationCallbacks *pAllocator )
{
    auto func = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(
        instance.getProcAddr( "vkDestroyDebugReportCallbackEXT" ) );

    if ( func != nullptr )
        func( static_cast<VkInstance>( instance ), callback, pAllocator );
}

rh::engine::VulkanRenderer::VulkanRenderer( HWND window, HINSTANCE inst )
    : IRenderer( window, inst )
{
    m_aExtensions.emplace_back( VK_KHR_SURFACE_EXTENSION_NAME );
    m_aExtensions.emplace_back( VK_KHR_WIN32_SURFACE_EXTENSION_NAME );
    m_aExtensions.emplace_back( VK_EXT_DEBUG_REPORT_EXTENSION_NAME );

    // This extension is crutial to display selection, but for some reason,
    // no implementation exists for windows, guess we'll have to live without it
    // :)
    // m_aExtensions.push_back(VK_KHR_DISPLAY_EXTENSION_NAME);
    //m_aLayers.emplace_back( "VK_LAYER_LUNARG_standard_validation" );
    m_aLayers.emplace_back( "VK_LAYER_LUNARG_monitor" );

    // App info
    vk::ApplicationInfo app_info{};
    app_info.pApplicationName = "Render Hook App";
    app_info.pEngineName = "Render Hook Engine";

    // Instance info
    vk::InstanceCreateInfo inst_info{};
    inst_info.pApplicationInfo = &app_info;
    inst_info.enabledExtensionCount = static_cast<uint32_t>( m_aExtensions.size() );
    inst_info.ppEnabledExtensionNames = m_aExtensions.data();
    inst_info.enabledLayerCount = static_cast<uint32_t>( m_aLayers.size() );
    inst_info.ppEnabledLayerNames = m_aLayers.data();

    // Create vulkan instance
    if ( !CALL_VK_API( vk::createInstance( &inst_info, nullptr, &m_vkInstance ),
                       TEXT( "VulkanRenderer failed to initialize: Failed to "
                             "initialize Vulkan Instance!" ) ) )
        return;

    // Enumerate GPUS
    m_aAdapters = m_vkInstance.enumeratePhysicalDevices();

    VkDebugReportCallbackCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT /* | VK_DEBUG_REPORT_WARNING_BIT_EXT
                       | VK_DEBUG_REPORT_INFORMATION_BIT_EXT
                       | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT*/
        ;
    createInfo.pfnCallback = debugCallback;
    if ( !CALL_VK_API( CreateDebugReportCallbackEXT( m_vkInstance, &createInfo, &m_debugCallback ),
                       TEXT( "VulkanRenderer failed to initialize: Failed to create "
                             "debug callback!" ) ) )
        return;

    SetCurrentAdapter( 0 );
    SetCurrentOutput( 0 );
}

rh::engine::VulkanRenderer::~VulkanRenderer()
{
    DestroyDebugReportCallbackEXT( m_vkInstance, m_debugCallback, nullptr );

    m_vkInstance.destroySurfaceKHR( m_vkWindowSurface );
    m_vkInstance.destroy();
}

bool rh::engine::VulkanRenderer::InitDevice()
{
    float queuePriority[] = {0.5f};
    const char *devExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    vk::DeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.queueFamilyIndex = m_aGraphicsQueueFamilies[0];
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = queuePriority;

    vk::DeviceCreateInfo info{};
    info.pQueueCreateInfos = &queueCreateInfo;
    info.queueCreateInfoCount = 1;
    info.enabledExtensionCount = 1;
    info.ppEnabledExtensionNames = devExtensions;

    m_vkDevice = m_aAdapters[m_uiCurrentAdapter].createDevice( info );

    // Retrieve queue handles
    m_vkGraphicsQueue = m_vkDevice.getQueue( m_aGraphicsQueueFamilies[0], 0 );
    //m_vkPresentQueue = m_vkDevice.getQueue( m_aGraphicsQueueFamilies[0], 1 );

    uint32_t backBufferImgCount = m_vkWindowSurfaceCap.minImageCount;
    m_vkMainCommandBufferList.resize( backBufferImgCount );

    // Initialize command pool and main command buffer
    vk::CommandPoolCreateInfo cmdPoolInfo{};
    cmdPoolInfo.queueFamilyIndex = m_aGraphicsQueueFamilies[0];
    cmdPoolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

    try {
        m_vkGraphicsQueueCommandPool = m_vkDevice.createCommandPool( cmdPoolInfo );

        vk::CommandBufferAllocateInfo mainCmdBufferInfo{};
        mainCmdBufferInfo.commandBufferCount = backBufferImgCount;
        mainCmdBufferInfo.commandPool = m_vkGraphicsQueueCommandPool;
        mainCmdBufferInfo.level = vk::CommandBufferLevel::ePrimary;

        try {
            m_vkMainCommandBufferList = m_vkDevice.allocateCommandBuffers( mainCmdBufferInfo );

            try {
                m_vkSwapChainImageSemaphore = m_vkDevice.createSemaphore( {} );

                try {
                    m_vkMainCmdBufferSemaphore = m_vkDevice.createSemaphore( {} );

                    m_vkMainCmdBufferFences.resize( backBufferImgCount );

                    vk::FenceCreateInfo renderFenceInfo{};
                    renderFenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

                    for ( size_t i = 0; i < m_vkMainCmdBufferFences.size(); i++ ) {
                        try {
                            m_vkMainCmdBufferFences[i] = m_vkDevice.createFence( renderFenceInfo );
                        } catch ( ... ) {
                            for ( size_t j = i + 1; j > 0; j-- )
                                m_vkDevice.destroyFence( m_vkMainCmdBufferFences[i - 1] );
                            m_vkDevice.destroySemaphore( m_vkMainCmdBufferSemaphore );
                            throw;
                        }
                    }

                    m_vkDevice.resetFences( m_vkMainCmdBufferFences );
                    vk::SwapchainCreateInfoKHR swapChainInfo = {};
                    swapChainInfo.surface = m_vkWindowSurface;
                    swapChainInfo.minImageCount = backBufferImgCount;
                    swapChainInfo.imageFormat = m_aWindowSurfaceFormats[0].format;
                    swapChainInfo.imageColorSpace = m_aWindowSurfaceFormats[0].colorSpace;
                    swapChainInfo.imageExtent = m_vkWindowSurfaceCap.currentExtent;
                    swapChainInfo.imageArrayLayers = 1;
                    swapChainInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment
                                               | ( m_vkWindowSurfaceCap.supportedUsageFlags
                                                   & vk::ImageUsageFlagBits::eTransferDst );
                    swapChainInfo.presentMode
                        = vk::PresentModeKHR::eMailbox; // m_aWindowSurfacePresentModes[2];
                    swapChainInfo.imageSharingMode = vk::SharingMode::eExclusive;
                    swapChainInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::
                        eOpaque; // VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;
                    swapChainInfo.preTransform = vk::SurfaceTransformFlagBitsKHR::
                        eIdentity; // m_vkWindowSurfaceCap.currentTransform;

                    try {
                        m_vkSwapChain = m_vkDevice.createSwapchainKHR( swapChainInfo );
                    } catch ( ... ) {
                        for ( auto m_vkMainCmdBufferFence : m_vkMainCmdBufferFences )
                            m_vkDevice.destroyFence( m_vkMainCmdBufferFence );
                        m_vkDevice.destroySemaphore( m_vkMainCmdBufferSemaphore );
                        throw;
                    }
                } catch ( ... ) {
                    m_vkDevice.destroySemaphore( m_vkSwapChainImageSemaphore );
                    throw;
                }
            } catch ( ... ) {
                m_vkDevice.freeCommandBuffers( m_vkGraphicsQueueCommandPool,
                                               m_vkMainCommandBufferList );
                throw;
            }
        } catch ( ... ) {
            m_vkDevice.destroyCommandPool( m_vkGraphicsQueueCommandPool );
            throw;
        }
    } catch ( ... ) {
        m_vkDevice.destroy();
        return false;
    }

    return true;
}

bool rh::engine::VulkanRenderer::ShutdownDevice()
{
    m_vkDevice.waitIdle();
    m_vkDevice.destroySwapchainKHR( m_vkSwapChain );

    for ( auto m_vkMainCmdBufferFence : m_vkMainCmdBufferFences )
        m_vkDevice.destroyFence( m_vkMainCmdBufferFence );

    m_vkDevice.destroySemaphore( m_vkMainCmdBufferSemaphore );
    m_vkDevice.destroySemaphore( m_vkSwapChainImageSemaphore );

    m_vkDevice.freeCommandBuffers( m_vkGraphicsQueueCommandPool, m_vkMainCommandBufferList );
    m_vkDevice.destroyCommandPool( m_vkGraphicsQueueCommandPool );

    m_vkDevice.destroy();

    return true;
}

bool rh::engine::VulkanRenderer::GetAdaptersCount( unsigned int &n )
{
    n = static_cast<UINT>( m_aAdapters.size() );

    return true;
}

bool rh::engine::VulkanRenderer::GetAdapterInfo( unsigned int n, rh::engine::String &info )
{
    if ( n >= m_aAdapters.size() )
        return false;

    info = ToRHString( m_aAdapters[n].getProperties().deviceName );

    return true;
}

bool rh::engine::VulkanRenderer::SetCurrentAdapter( unsigned int n )
{
    if ( n >= m_aAdapters.size() )
        return false;

    m_uiCurrentAdapter = n;

    // Retrieve Queue Families
    std::vector<vk::QueueFamilyProperties> queueFamilies = m_aAdapters[m_uiCurrentAdapter]
                                                               .getQueueFamilyProperties();
    uint32_t q = 0;

    // Initialize queue families
    for ( const auto &fam : queueFamilies ) {
        if ( fam.queueFlags & vk::QueueFlagBits::eGraphics )
            m_aGraphicsQueueFamilies.push_back( q );
        if ( fam.queueFlags & vk::QueueFlagBits::eCompute )
            m_aComputeQueueFamilies.push_back( q );
        if ( fam.queueFlags & vk::QueueFlagBits::eTransfer )
            m_aTransferQueueFamilies.push_back( q );
        q++;
    }

    if ( m_aGraphicsQueueFamilies.empty() )
        return false;

    // Retrieve window surface
    vk::Win32SurfaceCreateInfoKHR info{};
    info.hinstance = m_hInst;
    info.hwnd = m_hWnd;

    m_vkWindowSurface = m_vkInstance.createWin32SurfaceKHR( info );

    VkBool32 surfaceSupported = m_aAdapters[n].getSurfaceSupportKHR( m_aGraphicsQueueFamilies[0],
                                                                     m_vkWindowSurface );

    if ( !surfaceSupported )
        return false;

    m_vkWindowSurfaceCap = m_aAdapters[n].getSurfaceCapabilitiesKHR( m_vkWindowSurface );
    m_aWindowSurfaceFormats = m_aAdapters[n].getSurfaceFormatsKHR( m_vkWindowSurface );
    m_aWindowSurfacePresentModes = m_aAdapters[n].getSurfacePresentModesKHR( m_vkWindowSurface );

    return true;
}

bool rh::engine::VulkanRenderer::GetOutputCount( unsigned int adapterId, int &c )
{
    if ( adapterId >= m_aAdapters.size() )
        return false;
    // Enumerate outputs(displays)
    int32_t displayCount = 1;
    //m_aAdapters[c].get
    /*if (!CALL_VK_API(vkGetPhysicalDeviceDisplayPropertiesKHR(m_aAdapters[n],
&displayCount, nullptr), L"Failed to get output count!")) return false;
m_aOutputProperties.resize(displayCount);
if (!CALL_VK_API(vkGetPhysicalDeviceDisplayPropertiesKHR(m_aAdapters[n],
&displayCount, m_aOutputProperties.data()), L"Failed to get output count!"))
return false;*/
    c = displayCount;
    return true;
}

bool rh::engine::VulkanRenderer::GetOutputInfo( unsigned int n, std::wstring &info )
{
    if ( n >= m_aOutputProperties.size() )
        return false;

    info = L"DefaultDisplay";
    return true;
}

bool rh::engine::VulkanRenderer::SetCurrentOutput( unsigned int /*id*/ )
{
    return true;
}

bool rh::engine::VulkanRenderer::GetDisplayModeCount( unsigned int /*outputId*/, int &c )
{
    // Vulkan can't do such computations, so we fallback for now
    c = 1;
    return true;
}

bool rh::engine::VulkanRenderer::GetDisplayModeInfo( unsigned int /*id*/, DisplayModeInfo &info )
{
    info.width = 640;
    info.height = 480;
    info.refreshRate = 60;
    return true;
}

bool rh::engine::VulkanRenderer::SetCurrentDisplayMode( unsigned int /*id*/ )
{
    return true;
}

bool rh::engine::VulkanRenderer::GetCurrentAdapter( int &n )
{
    n = 0;
    return true;
}

bool rh::engine::VulkanRenderer::GetCurrentOutput( int &n )
{
    n = 0;
    return true;
}

bool rh::engine::VulkanRenderer::GetCurrentDisplayMode( int &n )
{
    n = 0;
    return true;
}

bool rh::engine::VulkanRenderer::Present( void *image )
{
    auto *backBuffer = reinterpret_cast<VulkanBackBuffer *>( image );

    if ( backBuffer == nullptr )
        return false;

    uint32_t currentBackBufferId = backBuffer->GetBackBufferID();

    vk::PresentInfoKHR presentInfo{};
    presentInfo.pSwapchains = &m_vkSwapChain;
    presentInfo.swapchainCount = 1;
    presentInfo.pImageIndices = &currentBackBufferId;
    presentInfo.pWaitSemaphores = &m_vkMainCmdBufferSemaphore;
    presentInfo.waitSemaphoreCount = 1;

    if ( m_vkGraphicsQueue.presentKHR( presentInfo ) == vk::Result::eErrorOutOfDateKHR ) {
        rh::debug::DebugLogger::Error( "UNHANDLED OUTOFDATEKHR" );
    }

    m_vkDevice.waitForFences( {m_vkMainCmdBufferFences[m_nCurrentMainCmdBuffer]}, true, UINT64_MAX );
    m_vkDevice.resetFences( {m_vkMainCmdBufferFences[m_nCurrentMainCmdBuffer]} );

    return true;
}

void *rh::engine::VulkanRenderer::GetCurrentDevice()
{
    return reinterpret_cast<void *>( &m_vkDevice );
}

void *rh::engine::VulkanRenderer::GetCurrentContext()
{
    return reinterpret_cast<void *>( &m_vkMainCommandBufferList[m_nCurrentMainCmdBuffer] );
}

void rh::engine::VulkanRenderer::BindViewPorts( const std::vector<ViewPort> & /*viewports*/ ) {}

void *rh::engine::VulkanRenderer::AllocateImageBuffer( const ImageBufferInfo &info )
{
    switch ( info.type ) {
    case rh::engine::ImageBufferType::Unknown:
        break;
    case rh::engine::ImageBufferType::BackBuffer:
        return new VulkanBackBuffer( m_vkDevice, m_vkSwapChain );
    case rh::engine::ImageBufferType::TextureBuffer:
        break;
    case rh::engine::ImageBufferType::DepthBuffer:
        break;
    case rh::engine::ImageBufferType::RenderTargetBuffer:
    case rh::engine::ImageBufferType::DynamicTextureArrayBuffer:
        break;
    }
    return nullptr;
}

bool rh::engine::VulkanRenderer::FreeImageBuffer( void *buffer, ImageBufferType type )
{
    switch ( type ) {
    case rh::engine::ImageBufferType::Unknown:
        break;
    case rh::engine::ImageBufferType::BackBuffer:
        delete static_cast<VulkanBackBuffer *>( buffer );
        break;
    case rh::engine::ImageBufferType::TextureBuffer:
        break;
    case rh::engine::ImageBufferType::DepthBuffer:
        break;
    case rh::engine::ImageBufferType::RenderTargetBuffer:
    case rh::engine::ImageBufferType::DynamicTextureArrayBuffer:
        break;
    }
    return true;
}

bool rh::engine::VulkanRenderer::BindImageBuffers( ImageBindType /*bindType*/,
                                                   const std::vector<IndexPtrPair> &buffers )
{
    auto *backBuffer = reinterpret_cast<VulkanBackBuffer *>( buffers[0].ptr );

    if ( backBuffer == nullptr )
        return false;

    vk::ImageSubresourceRange subresRange{};
    subresRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    subresRange.levelCount = 1;
    subresRange.layerCount = 1;

    // A barrier to convert back-buffer image from read-only to write-only and
    // from undefined layout to transfer layout
    vk::ImageMemoryBarrier cmdlistBarrier{};
    cmdlistBarrier.image = backBuffer->GetImage();
    cmdlistBarrier.srcAccessMask = vk::AccessFlagBits::eMemoryRead;
    cmdlistBarrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
    cmdlistBarrier.oldLayout = vk::ImageLayout::eUndefined;
    cmdlistBarrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
    cmdlistBarrier.srcQueueFamilyIndex = m_aTransferQueueFamilies[0];
    cmdlistBarrier.dstQueueFamilyIndex = m_aTransferQueueFamilies[0];
    cmdlistBarrier.subresourceRange = subresRange;

    m_vkMainCommandBufferList[m_nCurrentMainCmdBuffer]
        .pipelineBarrier( vk::PipelineStageFlagBits::eTransfer,
                          vk::PipelineStageFlagBits::eTransfer,
                          {},
                          nullptr,
                          nullptr,
                          {cmdlistBarrier} );

    return true;
}

bool rh::engine::VulkanRenderer::ClearImageBuffer( ImageClearType clearType,
                                                   void *buffer,
                                                   const std::array<float, 4> &clearColor )
{
    auto *backBuffer = reinterpret_cast<VulkanBackBuffer *>( buffer );

    if ( backBuffer == nullptr )
        return false;

    vk::ClearColorValue vkClearColor{};
    vk::ImageSubresourceRange subresRange{};
    switch ( clearType ) {
    case rh::engine::ImageClearType::Color:

        subresRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        subresRange.levelCount = 1;
        subresRange.layerCount = 1;

        vkClearColor.float32[0] = clearColor[0];
        vkClearColor.float32[1] = clearColor[1];
        vkClearColor.float32[2] = clearColor[2];
        vkClearColor.float32[3] = clearColor[3];
        m_vkMainCommandBufferList[m_nCurrentMainCmdBuffer]
            .clearColorImage( backBuffer->GetImage(),
                              vk::ImageLayout::eTransferDstOptimal,
                              vkClearColor,
                              {subresRange} );

        return true;
    case rh::engine::ImageClearType::Depth:
        break;
    case rh::engine::ImageClearType::Stencil:
        break;
    case rh::engine::ImageClearType::DepthStencil:
        break;
    }
    return false;
}

bool rh::engine::VulkanRenderer::BeginCommandList( void *cmdList )
{
    if ( cmdList == nullptr ) {
        m_vkMainCommandBufferList[m_nCurrentMainCmdBuffer].reset( {} );

        vk::CommandBufferBeginInfo info{};
        info.flags = {};

        m_vkMainCommandBufferList[m_nCurrentMainCmdBuffer].begin( info );

        return true;
    }
    return false;
}

bool rh::engine::VulkanRenderer::EndCommandList( void *cmdList )
{
    if ( cmdList == nullptr ) {
        m_vkMainCommandBufferList[m_nCurrentMainCmdBuffer].end();

        vk::PipelineStageFlags flags = vk::PipelineStageFlagBits::eTransfer;

        vk::SubmitInfo submitInfo{};
        submitInfo.commandBufferCount = 1;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &m_vkSwapChainImageSemaphore;
        submitInfo.pSignalSemaphores = &m_vkMainCmdBufferSemaphore;
        submitInfo.pCommandBuffers = &m_vkMainCommandBufferList[m_nCurrentMainCmdBuffer];
        submitInfo.pWaitDstStageMask = &flags;

        m_vkGraphicsQueue.submit( {submitInfo}, m_vkMainCmdBufferFences[m_nCurrentMainCmdBuffer] );

        return true;
    }

    return false;
}

bool rh::engine::VulkanRenderer::RequestSwapChainImage( void *frameBuffer )
{
    auto *backBuffer = reinterpret_cast<VulkanBackBuffer *>( frameBuffer );

    if ( backBuffer == nullptr )
        return false;

    uint32_t currentBackBufferId;
    currentBackBufferId = m_vkDevice
                              .acquireNextImageKHR( m_vkSwapChain,
                                                    UINT64_MAX,
                                                    m_vkSwapChainImageSemaphore,
                                                    nullptr )
                              .value;

    m_nCurrentMainCmdBuffer = currentBackBufferId;
    backBuffer->SetBackBufferID( currentBackBufferId );

    return true;
}

bool rh::engine::VulkanRenderer::PresentSwapChainImage( void *frameBuffer )
{
    auto *backBuffer = reinterpret_cast<VulkanBackBuffer *>( frameBuffer );

    if ( backBuffer == nullptr )
        return false;

    auto backBufferImg = backBuffer->GetImage();

    vk::ImageMemoryBarrier presentationBarrier{};
    presentationBarrier.image = backBufferImg;
    presentationBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    presentationBarrier.dstAccessMask = vk::AccessFlagBits::eMemoryRead;
    presentationBarrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
    presentationBarrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
    presentationBarrier.srcQueueFamilyIndex = m_aTransferQueueFamilies[0];
    presentationBarrier.dstQueueFamilyIndex = m_aTransferQueueFamilies[0];

    vk::ImageSubresourceRange subresRange{};
    subresRange.levelCount = 1;
    subresRange.layerCount = 1;
    subresRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    presentationBarrier.subresourceRange = subresRange;

    // transform image to required layout
    m_vkMainCommandBufferList[m_nCurrentMainCmdBuffer]
        .pipelineBarrier( vk::PipelineStageFlagBits::eTransfer,
                          vk::PipelineStageFlagBits::eBottomOfPipe,
                          {},
                          nullptr,
                          nullptr,
                          {presentationBarrier} );

    return true;
}

void rh::engine::VulkanRenderer::FlushCache() {}

rh::engine::IGPUAllocator *rh::engine::VulkanRenderer::GetGPUAllocator()
{
    return nullptr;
}

void rh::engine::VulkanRenderer::InitImGUI() {}

void rh::engine::VulkanRenderer::ImGUIStartFrame()
{
    // ImGui_ImplVulkan_InitInfo
}

void rh::engine::VulkanRenderer::ImGUIRender() {}

void rh::engine::VulkanRenderer::ShutdownImGUI() {}
