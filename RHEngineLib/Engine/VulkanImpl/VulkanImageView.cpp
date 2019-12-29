#include "VulkanImageView.h"
using namespace rh::engine;

VulkanImageView::VulkanImageView( vk::ImageView view )
    : m_vkImageView( view ), m_bOwner( false )
{
}

VulkanImageView::VulkanImageView( vk::Device                   device,
                                  const ImageViewCreateParams &create_params )
    : m_vkDevice( device )
{
    vk::ImageViewCreateInfo create_info{};
    create_info.image                       = create_params.mImage;
    create_info.format                      = create_params.mFormat;
    create_info.viewType                    = vk::ImageViewType::e2D;
    create_info.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    create_info.subresourceRange.layerCount = 1;
    create_info.subresourceRange.levelCount = 1;
    m_vkImageView = m_vkDevice.createImageView( create_info );
}

VulkanImageView::~VulkanImageView()
{
    if ( m_bOwner )
        m_vkDevice.destroyImageView( m_vkImageView );
}