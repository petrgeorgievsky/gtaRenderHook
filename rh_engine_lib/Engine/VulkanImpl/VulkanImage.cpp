#include "VulkanImage.h"
#include "VulkanConvert.h"

#include <vk_mem_alloc.h>

using namespace rh::engine;
namespace rh::engine
{
constexpr vk::ImageType Convert( ImageDimensions dims )
{
    switch ( dims )
    {
    case ImageDimensions::d1D: return vk::ImageType::e1D;
    case ImageDimensions::d2D: return vk::ImageType::e2D;
    case ImageDimensions::d3D: return vk::ImageType::e3D;
    }
    terminate();
}

constexpr vk::ImageTiling Convert( ImageTiling tiling )
{
    switch ( tiling )
    {
    case ImageTiling::Linear: return vk::ImageTiling::eLinear;
    case ImageTiling::PlatformSpecific: return vk::ImageTiling::eOptimal;
    }
    terminate();
}

constexpr vk::SampleCountFlagBits Convert( ImageSampleCount tiling )
{
    switch ( tiling )
    {
    case ImageSampleCount::Sample1PPX: return vk::SampleCountFlagBits::e1;
    case ImageSampleCount::Sample2PPX: return vk::SampleCountFlagBits::e2;
    case ImageSampleCount::Sample4PPX: return vk::SampleCountFlagBits::e4;
    case ImageSampleCount::Sample8PPX: return vk::SampleCountFlagBits::e8;
    case ImageSampleCount::Sample16PPX: return vk::SampleCountFlagBits::e16;
    case ImageSampleCount::Sample32PPX: return vk::SampleCountFlagBits::e32;
    case ImageSampleCount::Sample64PPX: return vk::SampleCountFlagBits::e64;
    }
    terminate();
}

constexpr vk::ImageUsageFlags ConvertImageUsage( uint32_t flags )
{
    vk::ImageUsageFlags res_flags{};
    if ( flags & ImageBufferUsage::TransferSrc )
        res_flags |= vk::ImageUsageFlagBits::eTransferSrc;
    if ( flags & ImageBufferUsage::TransferDst )
        res_flags |= vk::ImageUsageFlagBits::eTransferDst;
    if ( flags & ImageBufferUsage::ColorAttachment )
        res_flags |= vk::ImageUsageFlagBits::eColorAttachment;
    if ( flags & ImageBufferUsage::DepthStencilAttachment )
        res_flags |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
    if ( flags & ImageBufferUsage::InputAttachment )
        res_flags |= vk::ImageUsageFlagBits::eInputAttachment;
    if ( flags & ImageBufferUsage::Sampled )
        res_flags |= vk::ImageUsageFlagBits::eSampled;
    if ( flags & ImageBufferUsage::Storage )
        res_flags |= vk::ImageUsageFlagBits::eStorage;
    return res_flags;
}
} // namespace rh::engine

rh::engine::VulkanImage::VulkanImage(
    const VulkanImageBufferCreateParams &create_params )
    : mDevice( create_params.mDevice )
{
    vk::ImageCreateInfo create_info_impl{};
    create_info_impl.imageType = Convert( create_params.mDimension );
    create_info_impl.tiling    = Convert( create_params.mTiling );
    create_info_impl.extent    = vk::Extent3D{
        create_params.mWidth, create_params.mHeight, create_params.mDepth };
    create_info_impl.format      = Convert( create_params.mFormat );
    create_info_impl.mipLevels   = create_params.mMipLevels;
    create_info_impl.arrayLayers = create_params.mArrayLayers;
    create_info_impl.samples     = Convert( create_params.mSampleCount );
    // TODO: add ability to use images as rendertargets etc.
    create_info_impl.usage         = ConvertImageUsage( create_params.mUsage );
    create_info_impl.initialLayout = vk::ImageLayout::eUndefined;

    mAllocator = create_params.mAllocator->GetImpl();
    VkImageCreateInfo       imageCreateInfo = create_info_impl;
    VmaAllocationCreateInfo allocInfo       = {};
    allocInfo.usage                         = VMA_MEMORY_USAGE_GPU_ONLY;
    VkImage img;
    vmaCreateImage( mAllocator, &imageCreateInfo, &allocInfo, &img,
                    &mAllocation, nullptr );

    mImage = img; // mDevice.createImage( create_info_impl );

    /// TODO: Add pools for texture memory
    /*VulkanMemoryAllocationInfo alloc{};
    alloc.mDeviceLocal  = true;
    alloc.mRequirements = mDevice.getImageMemoryRequirements( mImage );

    mImageMemory = create_params.mAllocator->AllocateDeviceMemory( alloc );

    mDevice.bindImageMemory( mImage, mImageMemory, {} );*/
}

VulkanImage::~VulkanImage()
{
    vmaDestroyImage( mAllocator, mImage, mAllocation );
    // mDevice.destroy( mImage );
    // mDevice.freeMemory( mImageMemory );
}
