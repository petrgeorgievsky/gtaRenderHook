#pragma once
#include "types/image_buffer_format.h"
#include <cstdint>

namespace rh::engine
{
class IImageView;
class ISyncPrimitive;

enum class VSyncType : uint32_t
{
    None,
    FullRefreshRate,
    HalfRefreshRate
};

struct SwapchainFrame
{
    IImageView       *mImageView;
    ImageBufferFormat mImageFormat;
    uint32_t          mImageId;
    uint32_t          mWidth;
    uint32_t          mHeight;
};

struct PresentationParams
{
    VSyncType mVsyncType = VSyncType::None;
    uint32_t  mBufferCount;
    bool      mWindowed;
    bool      mUseHDR;
};

class ISwapchain
{
  public:
    ISwapchain()          = default;
    virtual ~ISwapchain() = default;

    ISwapchain( const ISwapchain & )            = delete;
    ISwapchain( ISwapchain && )                 = delete;
    ISwapchain &operator=( const ISwapchain & ) = delete;
    ISwapchain &operator=( ISwapchain && )      = delete;

    virtual SwapchainFrame GetAvailableFrame( ISyncPrimitive *signal )      = 0;
    virtual bool Present( uint32_t swapchain_img, ISyncPrimitive *waitFor ) = 0;
};
} // namespace rh::engine
