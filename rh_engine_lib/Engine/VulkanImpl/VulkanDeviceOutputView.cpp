#include "VulkanDeviceOutputView.h"
#include "DebugUtils/DebugLogger.h"
#include "SyncPrimitives/VulkanGPUSyncPrimitive.h"
#include "VulkanImageView.h"

using namespace rh::engine;

VulkanDeviceOutputView::VulkanDeviceOutputView(
    HWND window, vk::Instance instance, vk::PhysicalDevice gpu,
    vk::Device device, vk::Queue queue, uint32_t graphicsQueueFamilyIdx )
    : m_vkInstance( instance ), m_vkDevice( device ), m_vkPresentQueue( queue ),
      m_vkGPU( gpu )
{
    vk::Win32SurfaceCreateInfoKHR window_info{};
    window_info.hinstance = GetModuleHandle( nullptr );
    window_info.hwnd      = window;
    m_vkWindowsSurface    = m_vkInstance.createWin32SurfaceKHR( window_info );

    if ( !m_vkGPU.getSurfaceSupportKHR( graphicsQueueFamilyIdx,
                                        m_vkWindowsSurface ) )
    {
        m_vkInstance.destroySurfaceKHR( m_vkWindowsSurface );
        throw std::runtime_error( "Unsupported surface!" );
    }

    vk::SurfaceFormatKHR fmt;
    CreateSwapchain( graphicsQueueFamilyIdx, fmt );

    m_vkSwapchainImages = m_vkDevice.getSwapchainImagesKHR( m_vkSwapChain );
    m_vkSwapchainImageViews.reserve( m_vkSwapchainImages.size() );

    for ( auto img : m_vkSwapchainImages )
    {
        ImageViewCreateParams create_info{};
        create_info.mImage  = img;
        create_info.mFormat = fmt.format;

        m_vkSwapchainImageViews.emplace_back(
            new VulkanImageView( m_vkDevice, create_info ) );
    }
}

void VulkanDeviceOutputView::CreateSwapchain(
    uint32_t graphicsQueueFamilyIdx, vk::SurfaceFormatKHR &swapchainFmt )
{
    auto surface_caps = m_vkGPU.getSurfaceCapabilitiesKHR( m_vkWindowsSurface );
    auto surface_formats = m_vkGPU.getSurfaceFormatsKHR( m_vkWindowsSurface );
    auto surface_present_modes =
        m_vkGPU.getSurfacePresentModesKHR( m_vkWindowsSurface );

    auto present_mode = vk::PresentModeKHR::eFifo;
    if ( std::any_of(
             surface_present_modes.begin(), surface_present_modes.end(),
             []( auto el ) { return el == vk::PresentModeKHR::eMailbox; } ) )
        present_mode = vk::PresentModeKHR::eMailbox;

    swapchainFmt = surface_formats[0];

    // Swapchain creation
    vk::SwapchainCreateInfoKHR swapchain_info{};
    swapchain_info.surface          = m_vkWindowsSurface;
    swapchain_info.minImageCount    = surface_caps.minImageCount;
    swapchain_info.imageFormat      = swapchainFmt.format;
    swapchain_info.imageColorSpace  = swapchainFmt.colorSpace;
    swapchain_info.imageExtent      = surface_caps.currentExtent;
    swapchain_info.imageArrayLayers = 1;
    swapchain_info.imageUsage       = vk::ImageUsageFlagBits::eColorAttachment;
    swapchain_info.queueFamilyIndexCount = 1;
    swapchain_info.pQueueFamilyIndices   = &graphicsQueueFamilyIdx;
    swapchain_info.preTransform          = surface_caps.currentTransform;
    swapchain_info.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    swapchain_info.presentMode    = present_mode;

    m_vkSwapChain = m_vkDevice.createSwapchainKHR( swapchain_info );
}

VulkanDeviceOutputView::~VulkanDeviceOutputView()
{
    for ( auto img_view : m_vkSwapchainImageViews )
        delete img_view;
    if ( m_vkSwapChain )
        m_vkDevice.destroySwapchainKHR( m_vkSwapChain );
    if ( m_vkWindowsSurface )
        m_vkInstance.destroySurfaceKHR( m_vkWindowsSurface );
}

uint32_t
VulkanDeviceOutputView::GetFreeSwapchainImage( ISyncPrimitive *signal_prim )
{
    auto vk_signal_prim = static_cast<VulkanGPUSyncPrimitive *>( signal_prim );

    auto res = m_vkDevice.acquireNextImageKHR( m_vkSwapChain, UINT64_MAX,
                                               *vk_signal_prim, nullptr );
    assert( res.result == vk::Result::eSuccess );
    return res.value;
}

bool VulkanDeviceOutputView::Present( uint32_t        swapchain_img,
                                      ISyncPrimitive *waitFor )
{
    vk::PresentInfoKHR present_info{};
    present_info.swapchainCount = 1;
    present_info.pSwapchains    = &m_vkSwapChain;
    if ( waitFor )
    {
        auto vk_wait_prim = static_cast<VulkanGPUSyncPrimitive *>( waitFor );
        auto vk_wait_prim_impl = static_cast<vk::Semaphore>( *vk_wait_prim );
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores    = &vk_wait_prim_impl;
    }
    present_info.pImageIndices = &swapchain_img;
    m_vkPresentQueue.presentKHR( present_info );
    return true;
}

bool VulkanDeviceOutputView::Present()
{
    vk::PresentInfoKHR present_info{};
    present_info.swapchainCount     = 1;
    present_info.pSwapchains        = &m_vkSwapChain;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores    = &m_vkImageAquireSemaphore;
    // present_info.pImageIndices = &m_uiSwapchainIdx;
    m_vkPresentQueue.presentKHR( present_info );
    return true;
}

bool rh::engine::VulkanDeviceOutputView::SetFullscreenFlag( bool flag )
{
    return flag;
}

IImageView *VulkanDeviceOutputView::GetImageView( uint32_t id )
{
    return m_vkSwapchainImageViews[id];
}
