#pragma once
#include <Engine/Common/IGPUResource.h>
#include <common.h>
namespace rh::engine
{
class VulkanBackBuffer : public IGPUResource
{
public:
    VulkanBackBuffer( const vk::Device &device, const vk::SwapchainKHR &swapChain );
    ~VulkanBackBuffer();
    [[nodiscard]] vk::ImageView GetImageView() const;
    [[nodiscard]] vk::Image GetImage() const;

    /*
            Sets current back-buffer(where we will be rendering) id
        */
    void SetBackBufferID( uint32_t id );

    /*
            Returns current back-buffer id
        */
    uint32_t GetBackBufferID();

private:
    // Represe
    std::vector<vk::Image> m_vkBackBufferImages;
    vk::ImageView m_vkImageView = nullptr;
    uint32_t m_uiBackBufferID = 0;
    };
};
