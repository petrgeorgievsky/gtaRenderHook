#include "VulkanSwapchain.h"
#include "SyncPrimitives/VulkanGPUSyncPrimitive.h"
#include "VulkanImageView.h"

using namespace rh::engine;

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

    // TODO: Add better present mode selection code
    auto select_present_mode = [&surface_present_modes](
                                   VSyncType vsync_type ) {
        auto present_mode_supported =
            [&surface_present_modes](
                vk::PresentModeKHR present_mode ) -> bool {
            return std::any_of(
                surface_present_modes.begin(), surface_present_modes.end(),
                [&present_mode]( auto el ) { return el == present_mode; } );
        };

        switch ( vsync_type )
        {
        case VSyncType::None:
            if ( present_mode_supported( vk::PresentModeKHR::eImmediate ) )
                return vk::PresentModeKHR::eImmediate;
            if ( present_mode_supported( vk::PresentModeKHR::eMailbox ) )
                return vk::PresentModeKHR::eMailbox;
        case VSyncType::HalfRefreshRate:
            if ( present_mode_supported( vk::PresentModeKHR::eFifoRelaxed ) )
                return vk::PresentModeKHR::eFifoRelaxed;

        default: return vk::PresentModeKHR::eFifo;
        }
    };

    // TODO: Add ability to select prefered swapchain fmt
    auto swapchain_fmt = surface_formats[0];
    auto present_mode =
        select_present_mode( create_params.mPresentParams.mVsyncType );
    auto buffer_count =
        std::min<>( std::max<>( surface_caps.minImageCount,
                                create_params.mPresentParams.mBufferCount ),
                    surface_caps.maxImageCount );

    // Swapchain creation
    vk::SwapchainCreateInfoKHR swapchain_info{};
    swapchain_info.surface          = create_params.mSurface;
    swapchain_info.minImageCount    = buffer_count;
    swapchain_info.imageFormat      = swapchain_fmt.format;
    swapchain_info.imageColorSpace  = swapchain_fmt.colorSpace;
    swapchain_info.imageExtent      = surface_caps.currentExtent;
    swapchain_info.imageArrayLayers = 1;
    swapchain_info.imageUsage       = vk::ImageUsageFlagBits::eColorAttachment;
    swapchain_info.queueFamilyIndexCount = 1;
    swapchain_info.pQueueFamilyIndices   = &create_params.mPresentQueueIdx;
    swapchain_info.preTransform          = surface_caps.currentTransform;
    swapchain_info.compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    swapchain_info.presentMode      = present_mode;
    swapchain_info.imageSharingMode = vk::SharingMode::eExclusive;

    m_vkSwapChain = m_vkDevice.createSwapchainKHR( swapchain_info );
    mWidth        = swapchain_info.imageExtent.width;
    mHeight       = swapchain_info.imageExtent.height;

    m_vkSwapchainImages = m_vkDevice.getSwapchainImagesKHR( m_vkSwapChain );
    m_vkSwapchainImageViews.reserve( m_vkSwapchainImages.size() );

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

SwapchainFrame VulkanSwapchain::GetAvaliableFrame( ISyncPrimitive *signal )
{

    auto vk_signal_prim = static_cast<VulkanGPUSyncPrimitive *>( signal );

    auto res = m_vkDevice.acquireNextImageKHR( m_vkSwapChain, UINT64_MAX,
                                               *vk_signal_prim, nullptr );
    assert( res.result == vk::Result::eSuccess );
    auto image_id = res.value;
    return { m_vkSwapchainImageViews[image_id], image_id, mWidth, mHeight };
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
    m_vkPresentQueue.presentKHR( present_info );
    return true;
}
