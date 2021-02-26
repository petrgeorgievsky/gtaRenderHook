//
// Created by peter on 17.02.2021.
//

#include "im3d_state_recorder.h"
#include "data_desc/immediate_mode/im_state.h"
#include <ipc/MemoryReader.h>
#include <ipc/MemoryWriter.h>

namespace rh::rw::engine
{

constexpr auto IM3D_VERTEX_COUNT_LIMIT = 100000;
constexpr auto IM3D_INDEX_COUNT_LIMIT  = 100000;

Im3DStateRecorder::Im3DStateRecorder( ImmediateState &im_state ) noexcept
    : ImState( im_state )
{
    VertexBuffer.resize( IM3D_VERTEX_COUNT_LIMIT );
    IndexBuffer.resize( IM3D_INDEX_COUNT_LIMIT );
    DrawCalls.resize( 4000 );
}

Im3DStateRecorder::~Im3DStateRecorder() noexcept = default;

void Im3DStateRecorder::Transform( RwIm3DVertex *vertices, uint32_t count,
                                   RwMatrix *                ltm,
                                   [[maybe_unused]] uint32_t flags )
{
    StashedVertices      = vertices;
    StashedVerticesCount = count;
    if ( ltm )
    {
        StashedWorldTransform = DirectX::XMFLOAT4X3{
            ltm->right.x, ltm->up.x, ltm->at.x, ltm->pos.x,
            ltm->right.y, ltm->up.y, ltm->at.y, ltm->pos.y,
            ltm->right.z, ltm->up.z, ltm->at.z, ltm->pos.z,
        };
    }
    else
    {
        DirectX::XMStoreFloat4x3(
            &StashedWorldTransform,
            DirectX::XMMatrixTranspose( DirectX::XMMatrixIdentity() ) );
    }
}

void Im3DStateRecorder::RenderPrimitive( RwPrimitiveType primType )
{
    auto &result_dc = DrawCalls[DrawCallCount];

    result_dc.IndexBufferOffset  = IndexCount;
    result_dc.VertexBufferOffset = VertexCount;
    assert( StashedVertices );

    CopyMemory( ( VertexBuffer.data() + VertexCount ), StashedVertices,
                StashedVerticesCount * sizeof( RwIm3DVertex ) );
    VertexCount += StashedVerticesCount;

    result_dc.IndexCount  = 0;
    result_dc.VertexCount = StashedVerticesCount;

    result_dc.RasterId       = ImState.Raster;
    result_dc.WorldTransform = StashedWorldTransform;
    result_dc.State          = { ImState.ColorBlendSrc, ImState.ColorBlendDst,
                        ImState.ColorBlendOp,  ImState.BlendEnable,
                        ImState.ZTestEnable,   ImState.ZWriteEnable,
                        ImState.StencilEnable };
    result_dc.State.PrimType = primType;

    DrawCallCount++;
}

void Im3DStateRecorder::RenderIndexedPrimitive( RwPrimitiveType primType,
                                                uint16_t *      indices,
                                                int32_t         numIndices )
{
    auto &result_dc = DrawCalls[DrawCallCount];

    result_dc.IndexBufferOffset  = IndexCount;
    result_dc.VertexBufferOffset = VertexCount;
    assert( indices );
    assert( StashedVertices );

    CopyMemory( ( IndexBuffer.data() + IndexCount ), indices,
                numIndices * sizeof( uint16_t ) );
    CopyMemory( ( VertexBuffer.data() + VertexCount ), StashedVertices,
                StashedVerticesCount * sizeof( RwIm3DVertex ) );
    VertexCount += StashedVerticesCount;
    IndexCount += numIndices;

    result_dc.IndexCount  = numIndices;
    result_dc.VertexCount = StashedVerticesCount;

    result_dc.RasterId       = ImState.Raster;
    result_dc.WorldTransform = StashedWorldTransform;
    result_dc.State          = { ImState.ColorBlendSrc, ImState.ColorBlendDst,
                        ImState.ColorBlendOp,  ImState.BlendEnable,
                        ImState.ZTestEnable,   ImState.ZWriteEnable,
                        ImState.StencilEnable };
    result_dc.State.PrimType = primType;

    DrawCallCount++;
}

uint64_t Im3DStateRecorder::Serialize( MemoryWriter &writer )
{
    // serialize index buffer
    uint64_t index_count = IndexCount;
    writer.Write( &index_count );
    if ( IndexCount > 0 )
        writer.Write( IndexBuffer.data(), IndexCount );

    // serialize vertex buffer
    uint64_t vertex_count = VertexCount;
    writer.Write( &vertex_count );
    if ( VertexCount <= 0 )
        return writer.Pos();
    writer.Write( VertexBuffer.data(), VertexCount );

    // serialize drawcalls
    uint64_t dc_count = DrawCallCount;
    writer.Write( &dc_count );
    if ( DrawCallCount <= 0 )
        return writer.Pos();
    writer.Write( DrawCalls.data(), DrawCallCount );

    return writer.Pos();
}

Im3DRenderState Im3DRenderState::Deserialize( MemoryReader &reader )
{
    Im3DRenderState result{};
    auto            idx_count = *reader.Read<uint64_t>();
    if ( idx_count > 0 )
    {
        result.IndexBuffer = { reader.Read<uint16_t>( idx_count ),
                               static_cast<size_t>( idx_count ) };
    }

    auto vtx_count = *reader.Read<uint64_t>();
    if ( vtx_count <= 0 )
        return result;
    result.VertexBuffer = { reader.Read<RwIm3DVertex>( vtx_count ),
                            static_cast<size_t>( vtx_count ) };

    auto dc_count = *reader.Read<uint64_t>();
    if ( dc_count <= 0 )
        return result;
    result.DrawCalls = { reader.Read<Im3DDrawCall>( dc_count ),
                         static_cast<size_t>( dc_count ) };
    return result;
}

void Im3DStateRecorder::Flush()
{
    DrawCallCount = 0;
    VertexCount   = 0;
    IndexCount    = 0;
}
} // namespace rh::rw::engine