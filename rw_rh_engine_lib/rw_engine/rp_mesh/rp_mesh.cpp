#include "rp_mesh.h"
#include "../rw_stream/rw_stream.h"
#include <array>
#include <common_headers.h>

namespace rh::rw::engine {
uint32_t g_nMeshSerialNum = 0;
bool readGeometryMesh( void *stream, RpGeometry *geometry ) noexcept
{
    if ( !RwStreamFindChunk( stream, rwID_EXTENSION, nullptr, nullptr ) )
        return false;
    if ( !RwStreamFindChunk( stream, rwID_BINMESHPLUGIN, nullptr, nullptr ) )
        return false;
    geometry->mesh = rpMeshRead( stream, geometry, &geometry->matList );
    return geometry->mesh != nullptr;
}

struct binMeshHeader
{
    RwUInt32 flags;
    RwUInt32 numMeshes;
    RwUInt32 totalIndicesInMesh;
};

struct binMesh
{
    RwUInt32 numIndices;
    RwInt32 matIndex;
};

constexpr int RWINDEXBUFFERSIZE = 256;
using RpIndexBuffer = std::array<RwUInt32, RWINDEXBUFFERSIZE>;

RpMeshHeader *rpMeshRead( void *stream,
                          const RpGeometry *geometry,
                          const RpMaterialList *matList ) noexcept
{
    binMeshHeader bmh{};
    RpMeshHeader *meshHeader;
    RwUInt32 size;

    /* Read in a header */
    if ( !RwStreamRead( stream, &bmh, sizeof( bmh ) ) ) {
        /* Failure */
        return nullptr;
    }

    /* Figure out the size (little bit of slack for alignment) */
    size = ( sizeof( RpMeshHeader ) + ( ( sizeof( RpMesh ) + 4 ) * bmh.numMeshes ) );

    if ( !( rpGEOMETRYNATIVE & geometry->flags ) ) {
        size += ( sizeof( RxVertexIndex ) * bmh.totalIndicesInMesh );
    }

    /* Allocate memory for the mesh */
    // RHDebug::DebugLogger::Log( "mesh sz:" + std::to_string( size ) );
    meshHeader = static_cast<RpMeshHeader *>( malloc( size ) );
    if ( !meshHeader )
        return nullptr;
    memset( meshHeader, 0, size );
    //_rpMeshHeaderCreate( size );
    if ( meshHeader ) {
        RpMesh *mesh = reinterpret_cast<RpMesh *>( meshHeader + 1 );
        RxVertexIndex *meshIndices = reinterpret_cast<RxVertexIndex *>( mesh + bmh.numMeshes );
        RwUInt32 numMeshes;

        meshHeader->flags = bmh.flags;
        meshHeader->numMeshes = static_cast<RwUInt16>( bmh.numMeshes );
        meshHeader->serialNum = static_cast<RwUInt16>( g_nMeshSerialNum );
        meshHeader->totalIndicesInMesh = static_cast<RwUInt32>( bmh.totalIndicesInMesh );
        meshHeader->firstMeshOffset = 0;

        g_nMeshSerialNum++;

        /* Then for each mesh... */
        numMeshes = meshHeader->numMeshes;
        while ( numMeshes-- > 0 ) {
            binMesh bm;

            /* ... do another header... */
            if ( NULL == RwStreamRead( stream, &bm, sizeof( bm ) ) ) {
                /* Failure */
                return nullptr;
            }

            mesh->numIndices = bm.numIndices;
            mesh->material = matList->materials[bm.matIndex];
            mesh->indices = meshIndices;

            if ( !( rpGEOMETRYNATIVE & geometry->flags ) ) {
                RwUInt32 remainingIndices;

                /* ...and all the indices */
                remainingIndices = mesh->numIndices;
                while ( remainingIndices > 0 ) {
                    RpIndexBuffer IndexBuffer;
                    RwInt32 *source = reinterpret_cast<RwInt32 *>( &IndexBuffer[0] );
                    RwUInt32 readIndices;

                    readIndices = ( remainingIndices < RWINDEXBUFFERSIZE ) ? ( remainingIndices )
                                                                           : ( RWINDEXBUFFERSIZE );

                    if ( !RwStreamRead( stream, source, sizeof( RwInt32 ) * readIndices ) ) {
                        /* Failure */
                        return nullptr;
                    }

                    remainingIndices -= readIndices;
                    while ( readIndices-- > 0 ) {
                        *meshIndices++ = static_cast<RxVertexIndex>( *source++ );
                    }
                }
            }

            mesh++;
        }
    }

    /* All done */
    return ( meshHeader );
}

} // namespace rw_rh_engine
