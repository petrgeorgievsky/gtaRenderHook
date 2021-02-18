#pragma once
#include "common_headers.h"
#include <vector>

#include <Engine/Common/ArrayProxy.h>

namespace rh
{

namespace rw::engine
{

struct Im3DState
{
    uint8_t ColorBlendSrc;
    uint8_t ColorBlendDst;
    uint8_t ColorBlendOp;
    uint8_t BlendEnable;

    uint8_t ZTestEnable;
    uint8_t ZWriteEnable;
    uint8_t StencilEnable;
    uint8_t PrimType;
};

struct Im3DDrawCall
{
    uint64_t            RasterId;
    uint32_t            VertexBufferOffset;
    uint32_t            VertexCount;
    uint32_t            IndexBufferOffset;
    uint32_t            IndexCount;
    DirectX::XMFLOAT4X3 WorldTransform;
    Im3DState           State;
};

struct ImmediateState;
class MemoryWriter;
class MemoryReader;

/**
 * Memory view into current frame im3d render state
 */
struct Im3DRenderState
{
    rh::engine::ArrayProxy<uint16_t>     IndexBuffer;
    rh::engine::ArrayProxy<RwIm3DVertex> VertexBuffer;
    rh::engine::ArrayProxy<Im3DDrawCall> DrawCalls;
    static Im3DRenderState               Deserialize( MemoryReader &reader );
};

/**
 * State recorder for immediate 3d render mode, records draw calls and dynamic
 * index/vertex data in memory buffers to be sent to im3d renderer
 */
class Im3DStateRecorder
{
  public:
    Im3DStateRecorder( ImmediateState &im_state ) noexcept;
    ~Im3DStateRecorder() noexcept;

    void Transform( RwIm3DVertex *vertices, uint32_t count, RwMatrix *ltm,
                    [[maybe_unused]] uint32_t flags );
    void RenderIndexedPrimitive( RwPrimitiveType primType, uint16_t *indices,
                                 int32_t numIndices );
    void RenderPrimitive( RwPrimitiveType primType );
    uint64_t Serialize( MemoryWriter &writer );
    void     Flush();

  private:
    ImmediateState &    ImState;
    RwIm3DVertex *      StashedVertices      = nullptr;
    uint32_t            StashedVerticesCount = 0;
    DirectX::XMFLOAT4X3 StashedWorldTransform{};

    std::vector<uint16_t>     IndexBuffer{};
    std::vector<RwIm3DVertex> VertexBuffer{};
    std::vector<Im3DDrawCall> DrawCalls{};

    uint32_t IndexCount    = 0;
    uint32_t VertexCount   = 0;
    uint32_t DrawCallCount = 0;
};

} // namespace rw::engine

} // namespace rh