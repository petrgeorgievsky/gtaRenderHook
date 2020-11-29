//
// Created by peter on 19.11.2020.
//

#include "im3d_backend.h"
#include "im2d_backend.h"
#include <Engine/Common/types/blend_op.h>
#include <render_loop.h>
namespace rh::rw::engine
{

constexpr auto VERTEX_COUNT_LIMIT = 100000;
constexpr auto INDEX_COUNT_LIMIT  = 100000;

Im3DClient::Im3DClient() noexcept
{
    mVertexBuffer.resize( VERTEX_COUNT_LIMIT );
    mIndexBuffer.resize( INDEX_COUNT_LIMIT );
    mDrawCalls.resize( 4000 );
    mCurrentRasterId = gEmptyTextureId;
    mCurrentState    = { (uint8_t)rh::engine::BlendOp::SrcAlpha,
                      (uint8_t)rh::engine::BlendOp::InvSrcAlpha,
                      0,
                      0,
                      0,
                      0,
                      0 };
}

void Im3DClient::Transform( RwIm3DVertex *vertices, uint32_t count,
                            RwMatrix *ltm, uint32_t flags )
{
    mStashedVertices      = vertices;
    mStashedVerticesCount = count;
    if ( ltm )
    {
        mStashedWorldTransform = DirectX::XMFLOAT4X3{
            ltm->right.x, ltm->up.x, ltm->at.x, ltm->pos.x,
            ltm->right.y, ltm->up.y, ltm->at.y, ltm->pos.y,
            ltm->right.z, ltm->up.z, ltm->at.z, ltm->pos.z,
        };
    }
    else
    {
        DirectX::XMStoreFloat4x3(
            &mStashedWorldTransform,
            DirectX::XMMatrixTranspose( DirectX::XMMatrixIdentity() ) );
    }
}
void Im3DClient::RenderIndexedPrimitive( RwPrimitiveType primType,
                                         uint16_t *indices, int32_t numIndices )
{
    mDrawCalls[mDrawCallCount].mIndexBufferOffset  = mIndexCount;
    mDrawCalls[mDrawCallCount].mVertexBufferOffset = mVertexCount;
    assert( indices );
    assert( mStashedVertices );

    CopyMemory( ( mIndexBuffer.data() + mIndexCount ), indices,
                numIndices * sizeof( uint16_t ) );
    CopyMemory( ( mVertexBuffer.data() + mVertexCount ), mStashedVertices,
                mStashedVerticesCount * sizeof( RwIm3DVertex ) );
    mVertexCount += mStashedVerticesCount;
    mIndexCount += numIndices;

    mDrawCalls[mDrawCallCount].mIndexCount  = numIndices;
    mDrawCalls[mDrawCallCount].mVertexCount = mStashedVerticesCount;

    mDrawCalls[mDrawCallCount].mRasterId        = mCurrentRasterId;
    mDrawCalls[mDrawCallCount].mWorldTransform  = mStashedWorldTransform;
    mDrawCalls[mDrawCallCount].mState           = mCurrentState;
    mDrawCalls[mDrawCallCount].mState.mPrimType = primType;

    mDrawCallCount++;
}

uint64_t Im3DClient::Serialize( MemoryWriter &writer )
{
    auto &block_end_offset = writer.Current<uint64_t>();
    writer.Skip( sizeof( uint64_t ) );

    // serialize index buffer
    uint64_t index_count = mIndexCount;
    writer.Write( &index_count );
    if ( mIndexCount > 0 )
    {
        writer.Write( mIndexBuffer.data(), mIndexCount );
    }

    // serialize vertex buffer
    uint64_t vertex_count = mVertexCount;
    writer.Write( &vertex_count );
    if ( mVertexCount <= 0 )
    {
        block_end_offset = writer.Pos();
        return writer.Pos();
    }
    writer.Write( mVertexBuffer.data(), mVertexCount );

    // serialize drawcalls
    uint64_t dc_count = mDrawCallCount;
    writer.Write( &dc_count );
    if ( mDrawCallCount <= 0 )
    {
        block_end_offset = writer.Pos();
        return writer.Pos();
    }
    writer.Write( mDrawCalls.data(), mDrawCallCount );

    block_end_offset = writer.Pos();
    return writer.Pos();
}

void Im3DClient::Flush()
{
    mDrawCallCount = 0;
    mVertexCount   = 0;
    mIndexCount    = 0;
}

void Im3DClient::SetRaster( uint64_t id ) { mCurrentRasterId = id; }
void Im3DClient::SetBlendEnable( uint8_t state )
{
    mCurrentState.mBlendEnable = state;
}
void Im3DClient::SetBlendSrc( uint8_t state )
{
    mCurrentState.mColorBlendSrc = state;
}
void Im3DClient::SetBlendDest( uint8_t state )
{
    mCurrentState.mColorBlendDst = state;
}
void Im3DClient::SetBlendOp( uint8_t state )
{
    mCurrentState.mColorBlendOp = state;
}
void Im3DClient::SetDepthEnable( uint8_t state )
{
    mCurrentState.mZTestEnable = state;
}
void Im3DClient::SetDepthWriteEnable( uint8_t state )
{
    mCurrentState.mZWriteEnable = state;
}

int32_t Im3DRenderIndexedPrimitive( RwPrimitiveType primType, uint16_t *indices,
                                    int32_t numIndices )
{
    EngineClient::gIm3DGlobals.RenderIndexedPrimitive( primType, indices,
                                                       numIndices );
    return 1;
}
void *Im3DTransform( void *pVerts, uint32_t numVerts, RwMatrix *ltm,
                     uint32_t flags )
{
    EngineClient::gIm3DGlobals.Transform( static_cast<RwIm3DVertex *>( pVerts ),
                                          numVerts, ltm, flags );
    return pVerts;
}
int32_t Im3DEnd() { return 1; }
int32_t Im3DRenderPrimitive( RwPrimitiveType primType ) { return 1; }
int32_t Im3DRenderLine( int32_t vert1, int32_t vert2 ) { return 1; }
int32_t Im3DRenderTriangle( int32_t vert1, int32_t vert2, int32_t vert3 )
{
    return 1;
}
} // namespace rh::rw::engine