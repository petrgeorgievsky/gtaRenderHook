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

class Im3DClient
{
  public:
    Im3DClient() noexcept;
    void Transform( RwIm3DVertex *vertices, uint32_t count, RwMatrix *ltm,
                    uint32_t flags );
    void RenderIndexedPrimitive( RwPrimitiveType primType, uint16_t *indices,
                                 int32_t numIndices );
    void RenderPrimitive( RwPrimitiveType primType );
    uint64_t Serialize( MemoryWriter &writer );
    void     Flush();

    void SetRaster( uint64_t id );
    /// Blend state
    void SetBlendEnable( uint8_t state );
    void SetBlendSrc( uint8_t state );
    void SetBlendDest( uint8_t state );
    void SetBlendOp( uint8_t state );
    void SetDepthEnable( uint8_t state );
    void SetDepthWriteEnable( uint8_t state );

  private:
    RwIm3DVertex *      mStashedVertices;
    uint32_t            mStashedVerticesCount;
    DirectX::XMFLOAT4X3 mStashedWorldTransform;

    std::vector<uint16_t>     mIndexBuffer;
    std::vector<RwIm3DVertex> mVertexBuffer;
    std::vector<Im3DDrawCall> mDrawCalls;

    uint32_t  mIndexCount;
    uint32_t  mVertexCount;
    uint32_t  mDrawCallCount;
    uint64_t  mCurrentRasterId;
    Im3DState mCurrentState;
};
} // namespace rh::rw::engine