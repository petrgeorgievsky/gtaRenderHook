#include "im2d_backend.h"
#include "../system_funcs/rw_device_system_globals.h"
#include "camera_backend.h"
#include "common_headers.h"
#include "ipc/MemoryWriter.h"
#include "raster_backend.h"
#include <Engine/Common/ArrayProxy.h>
#include <Engine/Common/IBuffer.h>
#include <Engine/Common/IDescriptorSet.h>
#include <Engine/Common/IDeviceState.h>
#include <Engine/Common/IPipeline.h>
#include <Engine/Common/types/blend_op.h>
#include <array>
#include <render_loop.h>
#include <unordered_map>
#include <vector>

using namespace rh::rw::engine;
using namespace rh::engine;

int32_t gCurrentTextureId = 0;

constexpr auto VERTEX_COUNT_LIMIT = 100000;
constexpr auto INDEX_COUNT_LIMIT  = 100000;

RwIm2DVertex *TriFanToTriList( RwIm2DVertex *vertices_in,
                               RwIm2DVertex *vertices_out,
                               int32_t &     vertex_count )
{
    uint32_t k = 0;
    for ( uint32_t i = 1; i < vertex_count - 1; i++ )
    {
        vertices_out[k++] = ( vertices_in[0] );
        vertices_out[k++] = ( vertices_in[i] );
        vertices_out[k++] = ( vertices_in[i + 1] );
    }
    vertex_count = k;
    return vertices_out;
}

int32_t rh::rw::engine::Im2DRenderPrimitiveFunction( RwPrimitiveType primType,
                                                     RwIm2DVertex *  vertices,
                                                     int32_t numVertices )
{
    if ( DeviceGlobals::DeviceGlobalsPtr->curCamera == nullptr )
        return 1;
    auto to_vertices = vertices;
    auto vert_count  = numVertices;
    if ( primType == RwPrimitiveType::rwPRIMTYPETRIFAN )
    {
        std::vector<RwIm2DVertex> vertices_2;
        vertices_2.resize( ( numVertices - 1 ) * 3 );

        // convert trifan to trilist
        to_vertices =
            TriFanToTriList( vertices, vertices_2.data(), vert_count );

        EngineClient::gIm2DGlobals.RecordDrawCall( to_vertices, vert_count );
    }
    else
        EngineClient::gIm2DGlobals.RecordDrawCall( to_vertices, vert_count );
    return 1;
}

int32_t rh::rw::engine::Im2DRenderIndexedPrimitiveFunction(
    RwPrimitiveType primType, RwIm2DVertex *vertices, int32_t numVertices,
    int16_t *indices, int32_t numIndices )
{
    if ( DeviceGlobals::DeviceGlobalsPtr->curCamera == nullptr )
        return 1;
    auto to_vertices = vertices;
    auto vert_count  = numVertices;
    if ( primType == RwPrimitiveType::rwPRIMTYPETRIFAN )
    {
        std::vector<RwIm2DVertex> vertices_2;
        vertices_2.resize( ( numVertices - 1 ) * 3 );

        to_vertices =
            TriFanToTriList( vertices, vertices_2.data(), vert_count );
        EngineClient::gIm2DGlobals.RecordDrawCall( to_vertices, vert_count,
                                                   indices, numIndices );
    }
    else
        EngineClient::gIm2DGlobals.RecordDrawCall( to_vertices, vert_count,
                                                   indices, numIndices );
    return 1;
}

void rh::rw::engine::Im2DSetRaster( RwRaster *raster )
{
    gCurrentTextureId = reinterpret_cast<int32_t>( raster );
    if ( gCurrentTextureId == 0 )
    {
        EngineClient::gIm2DGlobals.SetRaster( gEmptyTextureId );
        // TODO: Remove this from here
        EngineClient::gIm3DGlobals.SetRaster( gEmptyTextureId );
        return;
    }
    auto *backend_ext = GetBackendRasterExt( raster );
    // TODO: Remove this from here
    EngineClient::gIm3DGlobals.SetRaster( backend_ext->mImageId );

    EngineClient::gIm2DGlobals.SetRaster( backend_ext->mImageId );
    // weird hack to fix GTA 3 blend in menus
    // TODO: check for image alpha before doing this
    EngineClient::gIm2DGlobals.SetBlendEnable( true );
}

void Im2DClientGlobals::RecordDrawCall( RwIm2DVertex *vertices,
                                        int32_t       numVertices )
{
    mDrawCalls[mDrawCallCount].mVertexBufferOffset = mVertexCount;
    CopyMemory( ( mVertexBuffer.data() + mVertexCount ), vertices,
                numVertices * sizeof( RwIm2DVertex ) );
    mVertexCount += numVertices;
    mDrawCalls[mDrawCallCount].mVertexCount       = numVertices;
    mDrawCalls[mDrawCallCount].mRasterId          = mCurrentRasterId;
    mDrawCalls[mDrawCallCount].mIndexCount        = 0;
    mDrawCalls[mDrawCallCount].mIndexBufferOffset = 0;
    mDrawCalls[mDrawCallCount].mBlendState        = mCurrentState;
    mDrawCallCount++;
}

uint64_t Im2DClientGlobals::Serialize( MemoryWriter &writer )
{
    /// Serialize schema:
    /// uint64 block_end_offset
    /// uint64 frame_index_count
    /// int16 frame_indices[frame_index_count]
    /// uint64 frame_vertex_count
    /// RwIm2DVertex frame_vertices[frame_vertex_count]
    /// uint64 frame_draw_call_count
    /// Im2DDrawCall frame_drawcalls[frame_draw_call_count]
    //
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

void Im2DClientGlobals::Flush()
{
    mDrawCallCount = 0;
    mVertexCount   = 0;
    mIndexCount    = 0;
}

Im2DClientGlobals::Im2DClientGlobals() noexcept
{
    mVertexBuffer.resize( VERTEX_COUNT_LIMIT );
    mIndexBuffer.resize( INDEX_COUNT_LIMIT );
    mDrawCalls.resize( 4000 );
    mDrawCallCount   = 0;
    mVertexCount     = 0;
    mIndexCount      = 0;
    mCurrentRasterId = gEmptyTextureId;
    mCurrentState    = { (uint8_t)rh::engine::BlendOp::SrcAlpha,
                      (uint8_t)rh::engine::BlendOp::InvSrcAlpha,
                      0,
                      0,
                      0,
                      0,
                      0 };
}

void Im2DClientGlobals::SetRaster( uint64_t id ) { mCurrentRasterId = id; }

void Im2DClientGlobals::RecordDrawCall( RwIm2DVertex *vertices,
                                        int32_t numVertices, int16_t *indices,
                                        int32_t numIndices )
{
    mDrawCalls[mDrawCallCount].mIndexBufferOffset = mIndexCount;
    CopyMemory( ( mIndexBuffer.data() + mIndexCount ), indices,
                numIndices * sizeof( int16_t ) );
    mIndexCount += numIndices;
    mDrawCalls[mDrawCallCount].mVertexBufferOffset = mVertexCount;
    CopyMemory( ( mVertexBuffer.data() + mVertexCount ), vertices,
                numVertices * sizeof( RwIm2DVertex ) );
    mVertexCount += numVertices;
    mDrawCalls[mDrawCallCount].mVertexCount = numVertices;
    mDrawCalls[mDrawCallCount].mRasterId    = mCurrentRasterId;
    mDrawCalls[mDrawCallCount].mIndexCount  = numIndices;
    mDrawCalls[mDrawCallCount].mBlendState  = mCurrentState;
    mDrawCallCount++;
}
void Im2DClientGlobals::SetBlendEnable( uint8_t state )
{
    mCurrentState.mBlendEnable = state;
}
void Im2DClientGlobals::SetBlendSrc( uint8_t state )
{
    mCurrentState.mColorBlendSrc = state;
}
void Im2DClientGlobals::SetBlendDest( uint8_t state )
{
    mCurrentState.mColorBlendDst = state;
}
void Im2DClientGlobals::SetBlendOp( uint8_t state )
{
    mCurrentState.mColorBlendOp = state;
}
void Im2DClientGlobals::SetDepthEnable( uint8_t state )
{
    mCurrentState.mZTestEnable = state;
}
void Im2DClientGlobals::SetDepthWriteEnable( uint8_t state )
{
    mCurrentState.mZWriteEnable = state;
}
