//
// Created by peter on 16.02.2021.
//

#include "im2d_state_recorder.h"
#include "data_desc/immediate_mode/im_state.h"
#include <ipc/MemoryReader.h>
#include <ipc/MemoryWriter.h>

namespace rh::rw::engine
{
namespace
{
constexpr auto Im2DVertexCountLimit = 100000;
constexpr auto Im2DIndexCountLimit  = 100000;
} // namespace
Im2DStateRecorder::Im2DStateRecorder( ImmediateState &im_state ) noexcept
    : ImState{ im_state }
{
    VertexBuffer.resize( Im2DVertexCountLimit );
    IndexBuffer.resize( Im2DIndexCountLimit );
    DrawCalls.resize( 4000 );
    DrawCallCount = 0;
    VertexCount   = 0;
    IndexCount    = 0;
}

Im2DStateRecorder::~Im2DStateRecorder() noexcept = default;

void Im2DStateRecorder::RecordDrawCall( RwIm2DVertex *vertices,
                                        int32_t       num_vertices )
{
    auto &result_dc = DrawCalls[DrawCallCount];
    auto &im_state  = ImState;

    result_dc.VertexBufferOffset = VertexCount;
    CopyMemory( ( VertexBuffer.data() + VertexCount ), vertices,
                num_vertices * sizeof( RwIm2DVertex ) );
    VertexCount += num_vertices;

    result_dc.VertexCount       = num_vertices;
    result_dc.RasterId          = im_state.Raster;
    result_dc.IndexCount        = 0;
    result_dc.IndexBufferOffset = 0;
    result_dc.BlendState = { im_state.ColorBlendSrc, im_state.ColorBlendDst,
                             im_state.ColorBlendOp,  im_state.BlendEnable,
                             im_state.ZTestEnable,   im_state.ZWriteEnable,
                             im_state.StencilEnable };

    DrawCallCount++;
}

void Im2DStateRecorder::RecordDrawCall( RwIm2DVertex *vertices,
                                        int32_t num_vertices, int16_t *indices,
                                        int32_t num_indices )
{
    auto &result_dc = DrawCalls[DrawCallCount];
    auto &im_state  = ImState;

    result_dc.IndexBufferOffset = IndexCount;
    CopyMemory( ( IndexBuffer.data() + IndexCount ), indices,
                num_indices * sizeof( int16_t ) );
    IndexCount += num_indices;

    result_dc.VertexBufferOffset = VertexCount;
    CopyMemory( ( VertexBuffer.data() + VertexCount ), vertices,
                num_vertices * sizeof( RwIm2DVertex ) );
    VertexCount += num_vertices;

    result_dc.VertexCount = num_vertices;
    result_dc.RasterId    = im_state.Raster;
    result_dc.IndexCount  = num_indices;
    result_dc.BlendState  = { im_state.ColorBlendSrc, im_state.ColorBlendDst,
                             im_state.ColorBlendOp,  im_state.BlendEnable,
                             im_state.ZTestEnable,   im_state.ZWriteEnable,
                             im_state.StencilEnable };
    DrawCallCount++;
}

uint64_t Im2DStateRecorder::Serialize( MemoryWriter &writer )
{
    // serialize index buffer
    uint64_t index_count = IndexCount;
    writer.Write( &index_count );
    if ( index_count > 0 )
        writer.Write( IndexBuffer.data(), index_count );

    // serialize vertex buffer
    uint64_t vertex_count = VertexCount;
    writer.Write( &vertex_count );
    if ( vertex_count <= 0 )
        return writer.Pos();
    writer.Write( VertexBuffer.data(), vertex_count );

    // serialize drawcalls
    uint64_t dc_count = DrawCallCount;
    writer.Write( &dc_count );
    if ( dc_count <= 0 )
        return writer.Pos();

    writer.Write( DrawCalls.data(), dc_count );
    return writer.Pos();
}

Im2DRenderState Im2DRenderState::Deserialize( MemoryReader &reader )
{
    Im2DRenderState result{};
    auto            idx_count = *reader.Read<uint64_t>();
    if ( idx_count > 0 )
    {
        result.IndexBuffer = { reader.Read<uint16_t>( idx_count ),
                               static_cast<size_t>( idx_count ) };
    }

    auto vtx_count = *reader.Read<uint64_t>();
    if ( vtx_count <= 0 )
        return result;
    result.VertexBuffer = { reader.Read<RwIm2DVertex>( vtx_count ),
                            static_cast<size_t>( vtx_count ) };

    auto dc_count = *reader.Read<uint64_t>();
    if ( dc_count <= 0 )
        return result;
    result.DrawCalls = { reader.Read<Im2DDrawCall>( dc_count ),
                         static_cast<size_t>( dc_count ) };
    return result;
}

void Im2DStateRecorder::Flush()
{
    DrawCallCount = 0;
    VertexCount   = 0;
    IndexCount    = 0;
}
} // namespace rh::rw::engine
