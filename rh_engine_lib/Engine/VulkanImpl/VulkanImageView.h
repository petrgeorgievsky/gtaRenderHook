#pragma once
#include "Engine/Common/IImageView.h"
#include <common.h>

namespace rh::engine
{

struct ImageViewCreateParams
{
    vk::Image  mImage;
    vk::Format mFormat;
};

struct VulkanImageViewCreateInfo : ImageViewCreateInfo
{
    // Dependencies...
    vk::Device mDevice;
};

class VulkanImageView : public IImageView
{
  public:
    // VulkanImageView( vk::ImageView view );
    VulkanImageView( vk::Device                   device,
                     const ImageViewCreateParams &create_params );
    VulkanImageView( const VulkanImageViewCreateInfo &create_params );
    virtual ~VulkanImageView() override;

    VulkanImageView( const VulkanImageView &view ) = delete;

    operator vk::ImageView() { return m_vkImageView; }

  private:
    vk::ImageView m_vkImageView;
    vk::Device    m_vkDevice;
    bool          m_bOwner = true;
};
} // namespace rh::engine