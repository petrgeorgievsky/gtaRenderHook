#pragma once
#include "ArrayProxy.h"
#include "Engine/Common/types/image_layout.h"
#include "Engine/Common/types/memory_access_flags.h"
#include "Engine/Common/types/pipeline_stages.h"
#include "types/viewport.h"
#include <Engine/Common/types/pipeline_bind_point.h>
#include <vector>

namespace rh::engine
{
class ISyncPrimitive;
class IFrameBuffer;
class IRenderPass;
class IPipeline;
class IPipelineLayout;
class IBuffer;
class IDescriptorSet;
class IImageBuffer;

enum ClearValueType
{
    Color,
    Depth
};
struct ClearValue
{
    ClearValueType type;
    struct ClearColor
    {
        unsigned char r, g, b, a;
    } color;

    struct ClearDepthStencil
    {
        float   depth;
        uint8_t stencil;
    } depthStencil;
};

struct RenderPassBeginInfo
{
    IRenderPass *          m_pRenderPass;
    IFrameBuffer *         m_pFrameBuffer;
    ArrayProxy<ClearValue> m_aClearValues;
};

struct VertexBufferBinding
{
    IBuffer *mBuffer;
    uint32_t mOffset;
    uint32_t mStride;
};

enum IndexType
{
    i16,
    i32
};

struct Scissor
{
    uint32_t offset_x;
    uint32_t offset_y;
    uint32_t size_x;
    uint32_t size_y;
};

struct DescriptorSetBindInfo
{
    PipelineBindPoint mPipelineBindPoint = PipelineBindPoint::Graphics;
    IPipelineLayout * mPipelineLayout;
    uint32_t          mDescriptorSetsOffset;
    ArrayProxy<IDescriptorSet *> mDescriptorSets;
};

struct BufferRegion
{
    uint64_t mOffset;
    uint32_t mRowLength;
    uint32_t mRowCount;
};

struct ImageSubresourceRegion
{
    uint32_t mipLevel;
    uint32_t baseArrayLayer;
    uint32_t layerCount;
};

struct ImageSubresourceRange
{
    uint32_t baseMipLevel;
    uint32_t levelCount;
    uint32_t baseArrayLayer;
    uint32_t layerCount;
};

struct ImageRegion
{
    ImageSubresourceRegion mSubresource;
    int32_t                mOffsetX;
    int32_t                mOffsetY;
    int32_t                mOffsetZ;
    uint32_t               mExtentW;
    uint32_t               mExtentH;
    uint32_t               mExtentD = 1;
};

struct BufferToImageCopySubInfo
{
    BufferRegion mFrom;
    ImageRegion  mTo;
};

struct ImageToBufferCopySubInfo
{
    ImageRegion  mFrom;
    BufferRegion mTo;
};

struct ImageToBufferCopyInfo
{
    IBuffer *                            mBuffer;
    IImageBuffer *                       mImage;
    ImageLayout                          mImageLayout;
    ArrayProxy<ImageToBufferCopySubInfo> mRegions;
};

struct BufferToImageCopyInfo
{
    IBuffer *                            mBuffer;
    IImageBuffer *                       mImage;
    ImageLayout                          mImageLayout;
    ArrayProxy<BufferToImageCopySubInfo> mRegions;
};

struct ImageToImageCopySubInfo
{
    ImageRegion mSrc;
    ImageRegion mDest;
};

struct ImageToImageCopyInfo
{
    IImageBuffer *                      mSrc;
    IImageBuffer *                      mDst;
    ImageLayout                         mSrcLayout;
    ImageLayout                         mDstLayout;
    ArrayProxy<ImageToImageCopySubInfo> mRegions;
};

struct ImageMemoryBarrierInfo
{
    IImageBuffer *        mImage;
    ImageLayout           mSrcLayout;
    ImageLayout           mDstLayout;
    MemoryAccessFlags     mSrcMemoryAccess;
    MemoryAccessFlags     mDstMemoryAccess;
    ImageSubresourceRange mSubresRange;
};

struct MemoryBarrierInfo
{
    MemoryAccessFlags mSrcMemoryAccess;
    MemoryAccessFlags mDstMemoryAccess;
};

struct PipelineBarrierInfo
{
    PipelineStage                      mSrcStage;
    PipelineStage                      mDstStage;
    ArrayProxy<MemoryBarrierInfo>      mMemoryBarriers;
    ArrayProxy<ImageMemoryBarrierInfo> mImageMemoryBarriers;
};

class ICommandBuffer
{
  public:
    virtual ~ICommandBuffer()                                         = default;
    virtual void BeginRecord()                                        = 0;
    virtual void EndRecord()                                          = 0;
    virtual void BeginRenderPass( const RenderPassBeginInfo &params ) = 0;
    virtual void EndRenderPass()                                      = 0;

    virtual void SetViewports( uint32_t                    start_id,
                               const ArrayProxy<ViewPort> &viewports ) = 0;
    virtual void SetScissors( uint32_t                   start_id,
                              const ArrayProxy<Scissor> &scissors )    = 0;

    virtual void
    BindDescriptorSets( const DescriptorSetBindInfo &bind_info ) = 0;

    virtual void BindPipeline( IPipeline *pipeline ) = 0;
    virtual void
                 BindVertexBuffers( uint32_t                               start_id,
                                    const ArrayProxy<VertexBufferBinding> &buffers ) = 0;
    virtual void BindIndexBuffer( uint32_t offset, IBuffer *buffer,
                                  IndexType type ) = 0;

    virtual void Draw( uint32_t vertex_count, uint32_t instance_count,
                       uint32_t first_vertex, uint32_t first_instance ) = 0;
    virtual void DrawIndexed( uint32_t index_count, uint32_t instance_count,
                              uint32_t first_index, uint32_t first_vertex,
                              uint32_t first_instance )                 = 0;

    virtual void
    CopyBufferToImage( const BufferToImageCopyInfo &copy_info ) = 0;
    virtual void
    CopyImageToBuffer( const ImageToBufferCopyInfo &copy_info )            = 0;
    virtual void CopyImageToImage( const ImageToImageCopyInfo &copy_info ) = 0;

    virtual void PipelineBarrier( const PipelineBarrierInfo &info ) = 0;

    virtual ISyncPrimitive *ExecutionFinishedPrimitive() = 0;
};
} // namespace rh::engine
