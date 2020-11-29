#include "VulkanRenderer.h"
#include "CDebug.h"
#include "VulkanCommandBufferMgr.h"
#include "VulkanDevice.h"
#include "stdafx.h"

CVulkanRenderer::CVulkanRenderer( HWND &window )
{
    m_layers.push_back( "VK_LAYER_LUNARG_standard_validation" );
    m_extensions.push_back( VK_EXT_DEBUG_REPORT_EXTENSION_NAME );
    m_extensions.push_back( VK_KHR_SURFACE_EXTENSION_NAME );
    m_extensions.push_back( VK_KHR_WIN32_SURFACE_EXTENSION_NAME );
    m_initInstance();
    m_initDebugCB();
    m_initWindowSurface( window );
}

CVulkanRenderer::~CVulkanRenderer()
{
    m_deInitWindowSurface();
    m_deInitDebugCB();
    m_deInitInstance();
}

void CVulkanRenderer::InitDevice()
{
    m_GPU_Properties    = getGPUProperties( m_GPU_ID );
    m_pDevice           = new CVulkanDevice( m_GPU_list[m_GPU_ID] );
    m_pCommandBufferMgr = new CVulkanCommandBufferMgr( m_pDevice );
    m_pSwapChain        = new CVulkanSwapChain( this );
    m_initRenderPass();
    m_pSwapChain->CreateFrameBuffers();
    m_initSemaphores();
}

void CVulkanRenderer::m_initRenderPass()
{
    VkAttachmentDescription attachment_descriptions[]     = { {
        0, // VkAttachmentDescriptionFlags   flags
        m_pSwapChain->getSurfaceFormat().format, // VkFormat format
        VK_SAMPLE_COUNT_1_BIT, // VkSampleCountFlagBits          samples
        VK_ATTACHMENT_LOAD_OP_CLEAR, // VkAttachmentLoadOp             loadOp
        VK_ATTACHMENT_STORE_OP_STORE, // VkAttachmentStoreOp            storeOp
        VK_ATTACHMENT_LOAD_OP_DONT_CARE, // VkAttachmentLoadOp stencilLoadOp
        VK_ATTACHMENT_STORE_OP_DONT_CARE, // VkAttachmentStoreOp stencilStoreOp
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, // VkImageLayout initialLayout;
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR  // VkImageLayout finalLayout
    } };
    VkAttachmentReference   color_attachment_references[] = { {
        0, // uint32_t                       attachment
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL // VkImageLayout layout
    } };

    VkSubpassDescription subpass_descriptions[] = { {
        0,                               // VkSubpassDescriptionFlags      flags
        VK_PIPELINE_BIND_POINT_GRAPHICS, // VkPipelineBindPoint
                                         // pipelineBindPoint
        0,       // uint32_t                       inputAttachmentCount
        nullptr, // const VkAttachmentReference   *pInputAttachments
        1,       // uint32_t                       colorAttachmentCount
        color_attachment_references, // const VkAttachmentReference
                                     // *pColorAttachments
        nullptr, // const VkAttachmentReference   *pResolveAttachments
        nullptr, // const VkAttachmentReference   *pDepthStencilAttachment
        0,       // uint32_t                       preserveAttachmentCount
        nullptr  // const uint32_t*                pPreserveAttachments
    } };

    VkRenderPassCreateInfo render_pass_create_info = {
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, // VkStructureType sType
        nullptr, // const void                    *pNext
        0,       // VkRenderPassCreateFlags        flags
        1,       // uint32_t                       attachmentCount
        attachment_descriptions, // const VkAttachmentDescription *pAttachments
        1,                       // uint32_t                       subpassCount
        subpass_descriptions,    // const VkSubpassDescription    *pSubpasses
        0,      // uint32_t                       dependencyCount
        nullptr // const VkSubpassDependency     *pDependencies
    };

    vkCreateRenderPass( m_pDevice->getDevice(), &render_pass_create_info,
                        nullptr, &m_renderPass );
}

void CVulkanRenderer::DeInitDevice()
{
    m_deInitSemaphores();
    delete m_pSwapChain;
    vkDestroyRenderPass( m_pDevice->getDevice(), m_renderPass, nullptr );
    delete m_pCommandBufferMgr;
    delete m_pDevice;
}

void CVulkanRenderer::BeginScene()
{

    vkAcquireNextImageKHR( m_pDevice->getDevice(), m_pSwapChain->getSwapChain(),
                           UINT64_MAX, m_imageAquiredSemaphore, VK_NULL_HANDLE,
                           &m_currentBuffer );
    m_pCommandBufferMgr->BeginRenderBuffer(
        m_pSwapChain->getImageList()[m_currentBuffer] );
}

void CVulkanRenderer::ClearScene( VkClearColorValue &clear_color )
{
    UNREFERENCED_PARAMETER( clear_color );
}

void CVulkanRenderer::EndScene()
{
    m_pCommandBufferMgr->EndRenderBuffer(
        m_pSwapChain->getImageList()[m_currentBuffer] );

    VkPipelineStageFlags wait_dst_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT;

    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType             = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    vkCreateFence( m_pDevice->getDevice(), &fenceCreateInfo, NULL,
                   &m_renderFence );

    VkSubmitInfo submitInfo{};
    submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pWaitDstStageMask    = &wait_dst_stage_mask;
    submitInfo.waitSemaphoreCount   = 1;
    submitInfo.pWaitSemaphores      = &m_imageAquiredSemaphore;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = &m_renderFinishSemaphore;
    submitInfo.commandBufferCount   = 1;
    submitInfo.pCommandBuffers = &m_pCommandBufferMgr->getRenderCommandBuffer();

    vkQueueSubmit( m_pDevice->getGraphicsQueue(), 1, &submitInfo,
                   m_renderFence );
    vkWaitForFences( m_pDevice->getDevice(), 1, &m_renderFence, VK_TRUE,
                     UINT64_MAX );
    vkDestroyFence( m_pDevice->getDevice(), m_renderFence, NULL );
}

void CVulkanRenderer::Present()
{
    VkPresentInfoKHR present{};
    present.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present.pNext              = NULL;
    present.swapchainCount     = 1;
    present.pSwapchains        = &m_pSwapChain->getSwapChain();
    present.pImageIndices      = &m_currentBuffer;
    present.waitSemaphoreCount = 1;
    present.pWaitSemaphores    = &m_renderFinishSemaphore;
    present.pResults           = NULL;

    vkQueuePresentKHR( m_pDevice->getGraphicsQueue(), &present );
}

void CVulkanRenderer::m_initInstance()
{
    VkApplicationInfo appInfo{};
    appInfo.sType      = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo        = &appInfo;
    instanceCreateInfo.enabledLayerCount       = m_layers.size();
    instanceCreateInfo.ppEnabledLayerNames     = m_layers.data();
    instanceCreateInfo.enabledExtensionCount   = m_extensions.size();
    instanceCreateInfo.ppEnabledExtensionNames = m_extensions.data();

    if ( vkCreateInstance( &instanceCreateInfo, nullptr, &m_Instance ) !=
         VK_SUCCESS )
        g_pDebug->printError( "Couldn't create an vulkan instance!" );
    // GPU Enumeration
    {
        uint32_t gpuCount;
        vkEnumeratePhysicalDevices( m_Instance, &gpuCount, nullptr );
        m_GPU_list.resize( gpuCount );
        vkEnumeratePhysicalDevices( m_Instance, &gpuCount, m_GPU_list.data() );
    }
    // Vulkan Instance Layer Enumeration
    {
        uint32_t layerPropCount;
        vkEnumerateInstanceLayerProperties( &layerPropCount, nullptr );
        std::vector<VkLayerProperties> layerPropList( layerPropCount );
        vkEnumerateInstanceLayerProperties( &layerPropCount,
                                            layerPropList.data() );
    }
    // Vulkan Device Layer Enumeration
    {
        uint32_t layerPropCount;
        vkEnumerateDeviceLayerProperties( m_GPU_list[m_GPU_ID], &layerPropCount,
                                          nullptr );
        std::vector<VkLayerProperties> layerPropList( layerPropCount );
        vkEnumerateDeviceLayerProperties( m_GPU_list[m_GPU_ID], &layerPropCount,
                                          layerPropList.data() );
    }
}

void CVulkanRenderer::m_deInitInstance()
{
    vkDestroyInstance( m_Instance, nullptr );
    m_Instance = VK_NULL_HANDLE;
}

void CVulkanRenderer::m_initDebugCB()
{
    VkDebugReportCallbackCreateInfoEXT debugCBinfo;
    debugCBinfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    debugCBinfo.pfnCallback = RwVKDebugCB;
    debugCBinfo.flags =
        /*VK_DEBUG_REPORT_INFORMATION_BIT_EXT | */
        VK_DEBUG_REPORT_WARNING_BIT_EXT |
        VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
        VK_DEBUG_REPORT_ERROR_BIT_EXT /*|	VK_DEBUG_REPORT_DEBUG_BIT_EXT*/;

    CreateDebugReportCallbackEXT( m_Instance, &debugCBinfo, nullptr,
                                  &m_debugCB );
}

void CVulkanRenderer::m_deInitDebugCB()
{
    DestroyDebugReportCallbackEXT( m_Instance, m_debugCB, nullptr );
    m_debugCB = VK_NULL_HANDLE;
}

void CVulkanRenderer::m_initWindowSurface( HWND &window )
{
    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo{};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.hinstance = GetModuleHandle( nullptr );
    surfaceCreateInfo.hwnd      = window;
    vkCreateWin32SurfaceKHR( m_Instance, &surfaceCreateInfo, nullptr,
                             &m_windowSurface );
}

void CVulkanRenderer::m_deInitWindowSurface()
{
    vkDestroySurfaceKHR( m_Instance, m_windowSurface, nullptr );
    m_windowSurface = VK_NULL_HANDLE;
}

void CVulkanRenderer::m_initSemaphores()
{
    VkSemaphoreCreateInfo imageAcquiredSemaphoreCreateInfo;
    imageAcquiredSemaphoreCreateInfo.sType =
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    imageAcquiredSemaphoreCreateInfo.pNext = NULL;
    imageAcquiredSemaphoreCreateInfo.flags = 0;

    vkCreateSemaphore( m_pDevice->getDevice(),
                       &imageAcquiredSemaphoreCreateInfo, NULL,
                       &m_imageAquiredSemaphore );
    vkCreateSemaphore( m_pDevice->getDevice(),
                       &imageAcquiredSemaphoreCreateInfo, NULL,
                       &m_renderFinishSemaphore );
}

void CVulkanRenderer::m_deInitSemaphores()
{
    vkDestroySemaphore( m_pDevice->getDevice(), m_renderFinishSemaphore, NULL );
    vkDestroySemaphore( m_pDevice->getDevice(), m_imageAquiredSemaphore, NULL );
}

void DestroyDebugReportCallbackEXT( VkInstance                   instance,
                                    VkDebugReportCallbackEXT     callback,
                                    const VkAllocationCallbacks *pAllocator )
{
    auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(
        instance, "vkDestroyDebugReportCallbackEXT" );
    if ( func != nullptr )
    {
        func( instance, callback, pAllocator );
    }
}

VkResult CreateDebugReportCallbackEXT(
    VkInstance instance, const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugReportCallbackEXT *   pCallback )
{
    auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(
        instance, "vkCreateDebugReportCallbackEXT" );
    if ( func != nullptr )
    {
        return func( instance, pCreateInfo, pAllocator, pCallback );
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL
RwVKDebugCB( VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
             uint64_t object, size_t location, int32_t messageCode,
             const char *pLayerPrefix, const char *pMessage, void *pUserData )
{
    UNREFERENCED_PARAMETER( flags );
    UNREFERENCED_PARAMETER( objectType );
    UNREFERENCED_PARAMETER( object );
    UNREFERENCED_PARAMETER( location );
    UNREFERENCED_PARAMETER( messageCode );
    UNREFERENCED_PARAMETER( pUserData );
    g_pDebug->printMsg( pLayerPrefix + std::string( " " ) + pMessage + "\n" );
    return VK_FALSE;
}
