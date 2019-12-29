#include "VulkanBackBuffer.h"
#include "../../../DebugUtils/DebugLogger.h"

rh::engine::VulkanBackBuffer::VulkanBackBuffer( const vk::Device &device, const vk::SwapchainKHR &swapChain )
{
    m_vkBackBufferImages = device.getSwapchainImagesKHR( swapChain );
}

rh::engine::VulkanBackBuffer::~VulkanBackBuffer()
= default;

vk::ImageView rh::engine::VulkanBackBuffer::GetImageView() const
{
    return vk::ImageView();
}

vk::Image rh::engine::VulkanBackBuffer::GetImage() const
{
    return m_vkBackBufferImages[m_uiBackBufferID];
}

uint32_t rh::engine::VulkanBackBuffer::GetBackBufferID()
{
    return m_uiBackBufferID;
}

void rh::engine::VulkanBackBuffer::SetBackBufferID( uint32_t id )
{
    m_uiBackBufferID = id;
}
