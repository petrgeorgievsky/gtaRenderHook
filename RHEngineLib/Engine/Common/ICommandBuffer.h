#pragma once
#include "types/viewport.h"
#include <vector>

namespace rh::engine
{
class ISyncPrimitive;
class IFrameBuffer;
class IRenderPass;
class IPipeline;
class IBuffer;
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
    IRenderPass *           m_pRenderPass;
    IFrameBuffer *          m_pFrameBuffer;
    std::vector<ClearValue> m_aClearValues;
};

struct VertexBufferBinding
{
    IBuffer *mBuffer;
    uint32_t mOffset;
};

struct Scissor
{
    uint32_t offset_x;
    uint32_t offset_y;
    uint32_t size_x;
    uint32_t size_y;
};

class ICommandBuffer
{
  public:
    virtual ~ICommandBuffer()                                         = default;
    virtual void BeginRecord()                                        = 0;
    virtual void EndRecord()                                          = 0;
    virtual void BeginRenderPass( const RenderPassBeginInfo &params ) = 0;
    virtual void EndRenderPass()                                      = 0;

    virtual void SetViewports( uint32_t                     start_id,
                               const std::vector<ViewPort> &viewports ) = 0;
    virtual void SetScissors( uint32_t                    start_id,
                              const std::vector<Scissor> &scissors )    = 0;

    virtual void BindPipeline( IPipeline *pipeline ) = 0;
    virtual void
    BindVertexBuffers( uint32_t                                start_id,
                       const std::vector<VertexBufferBinding> &buffers ) = 0;

    virtual void Draw( uint32_t vertex_count, uint32_t instance_count,
                       uint32_t first_vertex, uint32_t first_instance ) = 0;

    virtual ISyncPrimitive *ExecutionFinishedPrimitive() = 0;
};
} // namespace rh::engine