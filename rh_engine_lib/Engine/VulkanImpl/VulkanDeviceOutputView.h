#pragma once
#include "Engine/Common/IDeviceOutputView.h"
#include <common.h>

namespace rh::engine
{
class ISyncPrimitive;

/**
 * @brief Vulkan IDeviceOutputView implementation
 *
 * @usage:
 * auto output_view = {...};
 * while(...)
 * {
 *  output_view->ChangeWindowParams(...);
 *
 *  auto [frame_view, image_aquired] = output_view->GetFreeSwapchainImage();
 *
 *  auto dependency = dependent_op( image_aquired.await() );
 *
 *  output_view->Present( frame_view, dependency.await() );
 * }
 *
 * @see IDeviceOutputView for more info
 */
class VulkanDeviceOutputView : public IDeviceOutputView
{
    // IDeviceOutputView interface
  public:
    VulkanDeviceOutputView( HWND window, vk::Instance instance,
                            vk::PhysicalDevice gpu, vk::Device device,
                            vk::Queue queue, uint32_t graphicsQueueFamilyIdx );
    ~VulkanDeviceOutputView() override;

    uint32_t GetFreeSwapchainImage( ISyncPrimitive *signal_prim ) override;

    bool Present( uint32_t swapchain_img, ISyncPrimitive *waitFor ) override;

    bool Present() override;
    bool SetFullscreenFlag( bool flag ) override;

    IImageView *GetImageView( uint32_t id ) override;

  private:
    void                      CreateSwapchain( uint32_t              graphicsQueueFamilyIdx,
                                               vk::SurfaceFormatKHR &swapchainFmt );
    vk::PhysicalDevice        m_vkGPU                  = nullptr;
    vk::Semaphore             m_vkImageAquireSemaphore = nullptr;
    vk::Instance              m_vkInstance             = nullptr;
    vk::Device                m_vkDevice               = nullptr;
    vk::SwapchainKHR          m_vkSwapChain            = nullptr;
    vk::SurfaceKHR            m_vkWindowsSurface       = nullptr;
    vk::Queue                 m_vkPresentQueue         = nullptr;
    std::vector<vk::Image>    m_vkSwapchainImages{};
    std::vector<IImageView *> m_vkSwapchainImageViews{};
};
} // namespace rh::engine
