#pragma once
#include "Engine/Common/ISwapchain.h"
#include "Engine/Common/types/image_buffer_format.h"
#include <common.h>

namespace rh::engine
{

struct VulkanSwapchainCreateParams
{
    // Dependencies...
    vk::PhysicalDevice mGPU;
    vk::Device         mDevice;
    vk::Queue          mPresentQueue;
    uint32_t           mPresentQueueIdx;
    vk::SurfaceKHR     mSurface;
    // Present params
    PresentationParams mPresentParams;
};

class VulkanSwapchain : public ISwapchain
{
  public:
    VulkanSwapchain( const VulkanSwapchainCreateParams &create_params );
    ~VulkanSwapchain() override;
    SwapchainFrame GetAvailableFrame( ISyncPrimitive *signal ) override;
    bool Present( uint32_t swapchain_img, ISyncPrimitive *waitFor ) override;

  private:
    ImageBufferFormat         mSwapchainFormat;
    vk::SwapchainKHR          m_vkSwapChain    = nullptr;
    vk::Device                m_vkDevice       = nullptr;
    vk::Queue                 m_vkPresentQueue = nullptr;
    std::vector<vk::Image>    m_vkSwapchainImages{};
    std::vector<IImageView *> m_vkSwapchainImageViews{};
    uint32_t                  mWidth  = 0;
    uint32_t                  mHeight = 0;
};

} // namespace rh::engine
