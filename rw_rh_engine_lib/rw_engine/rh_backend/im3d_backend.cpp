//
// Created by peter on 19.11.2020.
//

#include "im3d_backend.h"
#include "im2d_backend.h"
#include "raster_backend.h"
#include <Engine/Common/types/blend_op.h>
#include <render_client/render_client.h>
#include <render_loop.h>
namespace rh::rw::engine
{

constexpr auto VERTEX_COUNT_LIMIT = 100000;
constexpr auto INDEX_COUNT_LIMIT  = 100000;

Im3DClient::Im3DClient( ImmediateState &im_state ) noexcept
    : ImState( im_state )
{
    mVertexBuffer.resize( VERTEX_COUNT_LIMIT );
    mIndexBuffer.resize( INDEX_COUNT_LIMIT );
    mDrawCalls.resize( 4000 );
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

    mDrawCalls[mDrawCallCount].mRasterId       = ImState.Raster;
    mDrawCalls[mDrawCallCount].mWorldTransform = mStashedWorldTransform;
    mDrawCalls[mDrawCallCount].mState          = {
        ImState.ColorBlendSrc, ImState.ColorBlendDst, ImState.ColorBlendOp,
        ImState.BlendEnable,   ImState.ZTestEnable,   ImState.ZWriteEnable,
        ImState.StencilEnable };
    mDrawCalls[mDrawCallCount].mState.mPrimType = primType;

    mDrawCallCount++;
}

uint64_t Im3DClient::Serialize( MemoryWriter &writer )
{
    // serialize index buffer
    uint64_t index_count = mIndexCount;
    writer.Write( &index_count );
    if ( mIndexCount > 0 )
        writer.Write( mIndexBuffer.data(), mIndexCount );

    // serialize vertex buffer
    uint64_t vertex_count = mVertexCount;
    writer.Write( &vertex_count );
    if ( mVertexCount <= 0 )
        return writer.Pos();
    writer.Write( mVertexBuffer.data(), mVertexCount );

    // serialize drawcalls
    uint64_t dc_count = mDrawCallCount;
    writer.Write( &dc_count );
    if ( mDrawCallCount <= 0 )
        return writer.Pos();
    writer.Write( mDrawCalls.data(), mDrawCallCount );

    return writer.Pos();
}

void Im3DClient::Flush()
{
    mDrawCallCount = 0;
    mVertexCount   = 0;
    mIndexCount    = 0;
}

void Im3DClient::RenderPrimitive( RwPrimitiveType primType )
{

    mDrawCalls[mDrawCallCount].mIndexBufferOffset  = mIndexCount;
    mDrawCalls[mDrawCallCount].mVertexBufferOffset = mVertexCount;
    assert( mStashedVertices );

    CopyMemory( ( mVertexBuffer.data() + mVertexCount ), mStashedVertices,
                mStashedVerticesCount * sizeof( RwIm3DVertex ) );
    mVertexCount += mStashedVerticesCount;

    mDrawCalls[mDrawCallCount].mIndexCount  = 0;
    mDrawCalls[mDrawCallCount].mVertexCount = mStashedVerticesCount;

    mDrawCalls[mDrawCallCount].mRasterId       = ImState.Raster;
    mDrawCalls[mDrawCallCount].mWorldTransform = mStashedWorldTransform;
    mDrawCalls[mDrawCallCount].mState          = {
        ImState.ColorBlendSrc, ImState.ColorBlendDst, ImState.ColorBlendOp,
        ImState.BlendEnable,   ImState.ZTestEnable,   ImState.ZWriteEnable,
        ImState.StencilEnable };
    mDrawCalls[mDrawCallCount].mState.mPrimType = primType;

    mDrawCallCount++;
}

int32_t Im3DRenderIndexedPrimitive( RwPrimitiveType primType, uint16_t *indices,
                                    int32_t numIndices )
{
    assert( gRenderClient );
    auto &im3d = gRenderClient->RenderState.Im3D;
    im3d.RenderIndexedPrimitive( primType, indices, numIndices );
    return 1;
}
void *Im3DTransform( void *pVerts, uint32_t numVerts, RwMatrix *ltm,
                     uint32_t flags )
{
    assert( gRenderClient );
    auto &im3d = gRenderClient->RenderState.Im3D;
    im3d.Transform( static_cast<RwIm3DVertex *>( pVerts ), numVerts, ltm,
                    flags );
    return pVerts;
}
int32_t Im3DEnd() { return 1; }
int32_t Im3DRenderPrimitive( RwPrimitiveType primType )
{
    assert( gRenderClient );
    auto &im3d = gRenderClient->RenderState.Im3D;
    im3d.RenderPrimitive( primType );
    return 1;
}
int32_t Im3DRenderLine( int32_t vert1, int32_t vert2 ) { return 1; }
int32_t Im3DRenderTriangle( int32_t vert1, int32_t vert2, int32_t vert3 )
{
    return 1;
}
} // namespace rh::rw::engine