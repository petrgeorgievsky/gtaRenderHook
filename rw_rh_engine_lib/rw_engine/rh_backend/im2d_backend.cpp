#include "im2d_backend.h"
#include "../system_funcs/rw_device_system_globals.h"
#include "common_headers.h"
#include "ipc/MemoryWriter.h"
#include "raster_backend.h"
#include <Engine/Common/IBuffer.h>
#include <Engine/Common/IDeviceState.h>
#include <array>
#include <render_client/render_client.h>
#include <vector>

namespace rh::rw::engine
{
using namespace rh::engine;

RwIm2DVertex *TriFanToTriList( RwIm2DVertex *vertices_in,
                               RwIm2DVertex *vertices_out,
                               int32_t &     vertex_count )
{
    uint32_t k = 0;
    for ( int32_t i = 1; i < vertex_count - 1; i++ )
    {
        vertices_out[k++] = ( vertices_in[0] );
        vertices_out[k++] = ( vertices_in[i] );
        vertices_out[k++] = ( vertices_in[i + 1] );
    }
    vertex_count = k;
    return vertices_out;
}

int32_t rh::rw::engine::Im2DRenderPrimitiveFunction( int32_t       primType,
                                                     RwIm2DVertex *vertices,
                                                     int32_t       numVertices )
{
    if ( gRwDeviceGlobals.DeviceGlobalsPtr->curCamera == nullptr )
        return 1;
    auto  to_vertices = vertices;
    auto  vert_count  = numVertices;
    auto &im2d        = gRenderClient->RenderState.Im2D;
    if ( primType == RwPrimitiveType::rwPRIMTYPETRIFAN )
    {
        std::vector<RwIm2DVertex> vertices_2;
        vertices_2.resize( ( numVertices - 1 ) * 3 );

        // convert trifan to trilist
        to_vertices =
            TriFanToTriList( vertices, vertices_2.data(), vert_count );

        im2d.RecordDrawCall( to_vertices, vert_count );
    }
    else
        im2d.RecordDrawCall( to_vertices, vert_count );
    return 1;
}

int32_t rh::rw::engine::Im2DRenderIndexedPrimitiveFunction(
    int32_t primType, RwIm2DVertex *vertices, int32_t numVertices,
    int16_t *indices, int32_t numIndices )
{
    if ( gRwDeviceGlobals.DeviceGlobalsPtr->curCamera == nullptr )
        return 1;
    auto  to_vertices = vertices;
    auto  vert_count  = numVertices;
    auto &im2d        = gRenderClient->RenderState.Im2D;
    if ( primType == RwPrimitiveType::rwPRIMTYPETRIFAN )
    {
        std::vector<RwIm2DVertex> vertices_2;
        vertices_2.resize( ( numVertices - 1 ) * 3 );

        to_vertices =
            TriFanToTriList( vertices, vertices_2.data(), vert_count );
        im2d.RecordDrawCall( to_vertices, vert_count, indices, numIndices );
    }
    else
        im2d.RecordDrawCall( to_vertices, vert_count, indices, numIndices );
    return 1;
}
} // namespace rh::rw::engine