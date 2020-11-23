#pragma once
#include "types\image_buffer_format.h"

namespace rh::engine
{
class IImageBuffer;
enum ImageViewUsage : int32_t
{
    ShaderResource     = 0b1,
    RenderTarget       = 0b10,
    RWTexture          = 0b100,
    DepthStencilTarget = 0b1000,
};

struct ImageViewCreateInfo
{
    IImageBuffer *    mBuffer;
    ImageBufferFormat mFormat;
    int32_t           mUsage;
    uint32_t          mBaseLevel  = 0;
    uint32_t          mLevelCount = 1;
};
class IImageView
{
  public:
    virtual ~IImageView() = default;
};
} // namespace rh::engine
