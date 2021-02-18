#pragma once
#include "common_headers.h"
#include <vector>

#include <Engine/Common/ArrayProxy.h>

struct RwIm2DVertex;

namespace rh
{

namespace rw::engine
{

struct Im2DState
{
    uint8_t mColorBlendSrc;
    uint8_t mColorBlendDst;
    uint8_t mColorBlendOp;
    uint8_t mBlendEnable;

    uint8_t mZTestEnable;
    uint8_t mZWriteEnable;
    uint8_t mStencilEnable;
};

struct Im2DDrawCall
{
    uint64_t mRasterId;
    uint32_t mVertexBufferOffset;
    uint32_t mVertexCount;
    uint32_t mIndexBufferOffset;
    uint32_t mIndexCount;
    // BlendState
    Im2DState mBlendState;
};

struct ImmediateState;
class MemoryWriter;
class MemoryReader;

struct Im2DRenderState
{
    rh::engine::ArrayProxy<uint16_t>     IndexBuffer;
    rh::engine::ArrayProxy<RwIm2DVertex> VertexBuffer;
    rh::engine::ArrayProxy<Im2DDrawCall> DrawCalls;
    static Im2DRenderState               Deserialize( MemoryReader &reader );
};

/**
 * State recorder for immediate 2d render mode, records draw calls and dynamic
 * index/vertex data in memory buffers to be sent to im2d renderer
 */
class Im2DStateRecorder
{
  public:
    Im2DStateRecorder( ImmediateState &im_state ) noexcept;
    ~Im2DStateRecorder() noexcept;

    void     RecordDrawCall( RwIm2DVertex *vertices, int32_t numVertices );
    void     RecordDrawCall( RwIm2DVertex *vertices, int32_t numVertices,
                             int16_t *indices, int32_t numIndices );
    uint64_t Serialize( MemoryWriter &writer );
    void     Flush();

  private:
    ImmediateState &          ImState;
    std::vector<Im2DDrawCall> DrawCalls{};
    std::vector<RwIm2DVertex> VertexBuffer{};
    std::vector<int16_t>      IndexBuffer{};
    uint32_t                  IndexCount    = 0;
    uint32_t                  VertexCount   = 0;
    uint32_t                  DrawCallCount = 0;
};

} // namespace rw::engine

} // namespace rh