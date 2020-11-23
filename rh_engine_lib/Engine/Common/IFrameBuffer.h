#pragma once
#include "ArrayProxy.h"
#include <cstdint>
#include <vector>

namespace rh::engine
{
class IRenderPass;
class IImageView;

struct FrameBufferCreateParams
{
    uint32_t                 width;
    uint32_t                 height;
    ArrayProxy<IImageView *> imageViews;
    IRenderPass *            renderPass;
};

struct FrameBufferInfo
{
    uint32_t width;
    uint32_t height;
};

class IFrameBuffer
{
  public:
    virtual ~IFrameBuffer()                        = default;
    virtual const FrameBufferInfo &GetInfo() const = 0;
};
} // namespace rh::engine