//
// Created by peter on 19.11.2020.
//

#pragma once
#include <vector>

#include <common_headers.h>
#include <ipc/MemoryWriter.h>
namespace rh::rw::engine
{
struct Im3DState
{
    uint8_t mColorBlendSrc;
    uint8_t mColorBlendDst;
    uint8_t mColorBlendOp;
    uint8_t mBlendEnable;

    uint8_t mZTestEnable;
    uint8_t mZWriteEnable;
    uint8_t mStencilEnable;
    uint8_t mPrimType;
};

struct Im3DDrawCall
{
    uint64_t            mRasterId;
    uint32_t            mVertexBufferOffset;
    uint32_t            mVertexCount;
    uint32_t            mIndexBufferOffset;
    uint32_t            mIndexCount;
    DirectX::XMFLOAT4X3 mWorldTransform;
    Im3DState           mState;
};

void *  Im3DTransform( void *pVerts, uint32_t numVerts, RwMatrix *ltm,
                       uint32_t flags );
int32_t Im3DRenderIndexedPrimitive( RwPrimitiveType primType, uint16_t *indices,
                                    int32_t numIndices );
int32_t Im3DRenderPrimitive( RwPrimitiveType primType );
int32_t Im3DRenderLine( int32_t vert1, int32_t vert2 );
int32_t Im3DRenderTriangle( int32_t vert1, int32_t vert2, int32_t vert3 );
int32_t Im3DEnd();

struct ImmediateState;
class Im3DClient
{
  public:
    Im3DClient( ImmediateState &im_state ) noexcept;
    void Transform( RwIm3DVertex *vertices, uint32_t count, RwMatrix *ltm,
                    uint32_t flags );
    void RenderIndexedPrimitive( RwPrimitiveType primType, uint16_t *indices,
                                 int32_t numIndices );
    void RenderPrimitive( RwPrimitiveType primType );
    uint64_t Serialize( MemoryWriter &writer );
    void     Flush();

  private:
    ImmediateState &    ImState;
    RwIm3DVertex *      mStashedVertices      = nullptr;
    uint32_t            mStashedVerticesCount = 0;
    DirectX::XMFLOAT4X3 mStashedWorldTransform{};

    std::vector<uint16_t>     mIndexBuffer{};
    std::vector<RwIm3DVertex> mVertexBuffer{};
    std::vector<Im3DDrawCall> mDrawCalls{};

    uint32_t mIndexCount    = 0;
    uint32_t mVertexCount   = 0;
    uint32_t mDrawCallCount = 0;
};
} // namespace rh::rw::engine