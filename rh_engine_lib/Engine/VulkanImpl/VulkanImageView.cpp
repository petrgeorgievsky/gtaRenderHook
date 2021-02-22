#include "VulkanImageView.h"
#include "VulkanConvert.h"
#include "VulkanImage.h"
#include <DebugUtils/DebugLogger.h>
using namespace rh::engine;
/*
VulkanImageView::VulkanImageView( vk::ImageView view )
    : m_vkImageView( view ), m_bOwner( false )
{
}*/

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
    
    auto result = m_vkDevice.createImageView( create_info );
    if ( result.result != vk::Result::eSuccess )
    {
        debug::DebugLogger::ErrorFmt( "Failed to create image view:%s",
                                      vk::to_string( result.result ).c_str() );
    }
    else
        m_vkImageView = result.value;
}

VulkanImageView::VulkanImageView(
    const VulkanImageViewCreateInfo &create_params )
    : m_vkDevice( create_params.mDevice ), m_bOwner( true )
{
    // TODO: REDO
    vk::ImageViewCreateInfo create_info{};
    create_info.image  = *dynamic_cast<VulkanImage *>( create_params.mBuffer );
    create_info.format = Convert( create_params.mFormat );
    create_info.viewType = vk::ImageViewType::e2D;
    create_info.subresourceRange.aspectMask =
        ( create_params.mUsage & ImageViewUsage::DepthStencilTarget ) ==
                ImageViewUsage::DepthStencilTarget
            ? vk::ImageAspectFlagBits::eDepth |
                  vk::ImageAspectFlagBits::eStencil
            : vk::ImageAspectFlagBits::eColor;
    create_info.subresourceRange.layerCount   = 1;
    create_info.subresourceRange.levelCount   = create_params.mLevelCount;
    create_info.subresourceRange.baseMipLevel = create_params.mBaseLevel;
    auto result = m_vkDevice.createImageView( create_info );

    if ( result.result != vk::Result::eSuccess )
    {
        debug::DebugLogger::ErrorFmt( "Failed to create image view:%s",
                                      vk::to_string( result.result ).c_str() );
    }
    else
        m_vkImageView = result.value;
}

VulkanImageView::~VulkanImageView()
{
    if ( m_bOwner )
        m_vkDevice.destroyImageView( m_vkImageView );
}