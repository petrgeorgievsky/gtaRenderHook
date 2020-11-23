#pragma once
#include "Engine\Common\IImageBuffer.h"
#include "VulkanMemoryAllocator.h"
#include <common.h>

VK_DEFINE_HANDLE( VmaAllocator )
VK_DEFINE_HANDLE( VmaAllocation )
namespace rh::engine
{

struct VulkanImageBufferCreateParams : ImageBufferCreateParams
{
    // Dependencies...
    vk::Device             mDevice;
    VulkanMemoryAllocator *mAllocator;
};

class VulkanImage : public IImageBuffer
{
  public:
    // VulkanImageView( vk::ImageView view );
    VulkanImage( const VulkanImageBufferCreateParams &create_params );
    ~VulkanImage() override;

    VulkanImage( const VulkanImage &img ) = delete;

    operator vk::Image() { return mImage; }

  private:
    vk::Image        mImage;
    vk::DeviceMemory mImageMemory;
    vk::Device       mDevice;
    VmaAllocator     mAllocator;
    VmaAllocation    mAllocation;
};
} // namespace rh::engine