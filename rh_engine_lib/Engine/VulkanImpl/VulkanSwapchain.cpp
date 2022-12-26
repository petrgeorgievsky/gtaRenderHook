#include "VulkanSwapchain.h"
#include "SyncPrimitives/VulkanGPUSyncPrimitive.h"
#include "VulkanCommon.h"
#include "VulkanConvert.h"
#include "VulkanImageView.h"

#include <ranges>

using namespace rh::engine;

namespace
{
vk::PresentModeKHR
SelectPresentMode( VSyncType                              vsync_type,
                   const std::vector<vk::PresentModeKHR> &supported_modes )
{
    auto supported = [&supported_modes]( vk::PresentModeKHR mode )
    {
        return std::ranges::any_of( supported_modes,
                                    [&mode]( auto el ) { return el == mode; } );
    };
    switch ( vsync_type )
    {
    case VSyncType::None:
        if ( supported( vk::PresentModeKHR::eImmediate ) )
            return vk::PresentModeKHR::eImmediate;
        if ( supported( vk::PresentModeKHR::eMailbox ) )
            return vk::PresentModeKHR::eMailbox;
    case VSyncType::HalfRefreshRate:
        if ( supported( vk::PresentModeKHR::eFifoRelaxed ) )
            return vk::PresentModeKHR::eFifoRelaxed;

    default: return vk::PresentModeKHR::eFifo;
    }
}

vk::SurfaceFormatKHR
SelectFormat( bool                                     prefer_hdr,
              const std::vector<vk::SurfaceFormatKHR> &supported_formats )
{
    if ( prefer_hdr )
    {
        auto hdr_fmt = std::ranges::find_if(
            supported_formats,
            []( VkSurfaceFormatKHR fmt )
            {
                return fmt.colorSpace ==
                       VkColorSpaceKHR::VK_COLOR_SPACE_HDR10_ST2084_EXT;
            } );
        if ( hdr_fmt != supported_formats.end() )
            return *hdr_fmt;
    }
    return supported_formats.front();
}

} // namespace

VulkanSwapchain::VulkanSwapchain(
    const VulkanSwapchainCreateParams &create_params )
    : m_vkDevice( create_params.mDevice ),
      m_vkPresentQueue( create_params.mPresentQueue )
{
    auto surface_caps =
        create_params.mGPU.getSurfaceCapabilitiesKHR( create_params.mSurface );
    auto surface_formats =
        create_params.mGPU.getSurfaceFormatsKHR( create_params.mSurface );
    auto surface_present_modes =
        create_params.mGPU.getSurfacePresentModesKHR( create_params.mSurface );

    auto swapchain_fmt = SelectFormat( create_params.mPresentParams.mUseHDR,
                                       surface_formats.value );
    auto present_mode  = SelectPresentMode(
        create_params.mPresentParams.mVsyncType, surface_present_modes.value );
    auto buffer_count =
        std::min<>( std::max<>( surface_caps.value.minImageCount,
                                create_params.mPresentParams.mBufferCount ),
                    surface_caps.value.maxImageCount );

    // Swapchain creation
    vk::SwapchainCreateInfoKHR swapchain_info{};
    swapchain_info.surface          = create_params.mSurface;
    swapchain_info.minImageCount    = buffer_count;
    swapchain_info.imageFormat      = swapchain_fmt.format;
    swapchain_info.imageColorSpace  = swapchain_fmt.colorSpace;
    swapchain_info.imageExtent      = surface_caps.value.currentExtent;
    swapchain_info.imageArrayLayers = 1;
    swapchain_info.imageUsage       = vk::ImageUsageFlagBits::eColorAttachment;
    swapchain_info.queueFamilyIndexCount = 1;
    swapchain_info.pQueueFamilyIndices   = &create_params.mPresentQueueIdx;
    swapchain_info.preTransform          = surface_caps.value.currentTransform;
    swapchain_info.compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    swapchain_info.presentMode      = present_mode;
    swapchain_info.imageSharingMode = vk::SharingMode::eExclusive;
    // vk::SurfaceFullScreenExclusiveInfoEXT fs_ex{};
    // fs_ex.fullScreenExclusive = vk::FullScreenExclusiveEXT::eDefault;
    // swapchain_info.setPNext( &fs_ex );
    auto swapchain_res = m_vkDevice.createSwapchainKHR( swapchain_info );
    if ( swapchain_res.result != vk::Result::eSuccess )
        debug::DebugLogger::ErrorFmt(
            "Failed to create swapchain:%s",
            vk::to_string( swapchain_res.result ).c_str() );
    else
        m_vkSwapChain = swapchain_res.value;
    // try to acquire exclusive fullscreen
    // if ( !create_params.mPresentParams.mWindowed )
    //    m_vkDevice.acquireFullScreenExclusiveModeEXT( m_vkSwapChain );

    mWidth  = swapchain_info.imageExtent.width;
    mHeight = swapchain_info.imageExtent.height;

    auto swapchain_img_res = m_vkDevice.getSwapchainImagesKHR( m_vkSwapChain );
    if ( swapchain_img_res.result != vk::Result::eSuccess )
        debug::DebugLogger::ErrorFmt(
            "Failed to retrieve swapchain images:%s",
            vk::to_string( swapchain_img_res.result ).c_str() );
    else
        m_vkSwapchainImages = swapchain_img_res.value;
    m_vkSwapchainImageViews.reserve( m_vkSwapchainImages.size() );

    mSwapchainFormat = Convert( swapchain_fmt.format );

    for ( auto img : m_vkSwapchainImages )
    {
        ImageViewCreateParams create_info{};
        create_info.mImage  = img;
        create_info.mFormat = swapchain_fmt.format;

        m_vkSwapchainImageViews.emplace_back(
            new VulkanImageView( m_vkDevice, create_info ) );
    }
}

VulkanSwapchain::~VulkanSwapchain()
{
    for ( auto img_view : m_vkSwapchainImageViews )
        delete img_view;
    if ( m_vkDevice )
        m_vkDevice.destroySwapchainKHR( m_vkSwapChain );
}

SwapchainFrame VulkanSwapchain::GetAvailableFrame( ISyncPrimitive *signal )
{
    auto vk_signal_prim = static_cast<VulkanGPUSyncPrimitive *>( signal );

    auto res = m_vkDevice.acquireNextImageKHR( m_vkSwapChain, UINT64_MAX,
                                               *vk_signal_prim, nullptr );
    assert( res.result == vk::Result::eSuccess );
    auto image_id = res.value;
    return { m_vkSwapchainImageViews[image_id], mSwapchainFormat, image_id,
             mWidth, mHeight };
}

bool VulkanSwapchain::Present( uint32_t swapchain_img, ISyncPrimitive *waitFor )
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
    auto result                = m_vkPresentQueue.presentKHR( present_info );
    return CALL_VK_API( result, "Failed to present image to surface" );
}
