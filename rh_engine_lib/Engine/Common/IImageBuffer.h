#pragma once
#include "ArrayProxy.h"
#include "types\image_buffer_format.h"
namespace rh::engine
{
enum class ImageDimensions
{
    d1D,
    d2D,
    d3D
};

enum class ImageTiling
{
    Linear,
    PlatformSpecific
};

enum class ImageSampleCount
{
    Sample1PPX,
    Sample2PPX,
    Sample4PPX,
    Sample8PPX,
    Sample16PPX,
    Sample32PPX,
    Sample64PPX,
};

enum ImageBufferUsage : uint32_t
{
    TransferSrc            = 0b1,
    TransferDst            = 0b10,
    Sampled                = 0b100,
    Storage                = 0b1000,
    ColorAttachment        = 0b10000,
    DepthStencilAttachment = 0b100000,
    InputAttachment        = 0b1000000,
};

struct ImageBufferInitData
{
    void *   mData;
    uint32_t mSize;
    uint32_t mStride;
};

struct ImageBufferCreateParams
{
    ImageDimensions   mDimension;
    ImageBufferFormat mFormat;
    uint32_t mUsage = ImageBufferUsage::TransferDst | ImageBufferUsage::Sampled;
    uint32_t mHeight;
    uint32_t mWidth;
    uint32_t mDepth                         = 1;
    uint32_t mMipLevels                     = 1;
    uint32_t mArrayLayers                   = 1;
    ImageTiling                     mTiling = ImageTiling::PlatformSpecific;
    ImageSampleCount                mSampleCount = ImageSampleCount::Sample1PPX;
    ArrayProxy<ImageBufferInitData> mPreinitData;
};

class IImageBuffer
{
  public:
    virtual ~IImageBuffer() = default;
};
} // namespace rh::engine