#include "rp_mesh.h"
#include "../rw_stream/rw_stream.h"
#include <array>
#include <common_headers.h>

namespace rh::rw::engine
{
uint32_t g_nMeshSerialNum = 0;
bool     readGeometryMesh( void *stream, RpGeometry *geometry ) noexcept
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
    uint32_t flags;
    uint32_t numMeshes;
    uint32_t totalIndicesInMesh;
};

struct binMesh
{
    uint32_t numIndices;
    int32_t  matIndex;
};

constexpr int RWINDEXBUFFERSIZE = 256;
using RpIndexBuffer             = std::array<uint32_t, RWINDEXBUFFERSIZE>;
enum RpGeometryFlag
{
    rpGEOMETRYTRISTRIP = 0x00000001,  /**<This geometry's meshes can be
                                          rendered as strips.
                                          \ref RpMeshSetTriStripMethod is
                                          used to change this method.*/
    rpGEOMETRYPOSITIONS = 0x00000002, /**<This geometry has positions */
    rpGEOMETRYTEXTURED  = 0x00000004, /**<This geometry has only one set of
                                          texture coordinates. Texture
                                          coordinates are specified on a per
                                          vertex basis */
    rpGEOMETRYPRELIT  = 0x00000008,   /**<This geometry has pre-light colors */
    rpGEOMETRYNORMALS = 0x00000010,   /**<This geometry has vertex normals */
    rpGEOMETRYLIGHT   = 0x00000020,   /**<This geometry will be lit */
    rpGEOMETRYMODULATEMATERIALCOLOR = 0x00000040, /**<Modulate material color
                                                      with vertex colors
                                                      (pre-lit + lit) */

    rpGEOMETRYTEXTURED2 = 0x00000080, /**<This geometry has at least 2 sets of
                                          texture coordinates. */

    /*
     * These above flags were stored in the flags field in an RwObject, they
     * are now stored in the flags file of the RpGeometry.
     */

    rpGEOMETRYNATIVE         = 0x01000000,
    rpGEOMETRYNATIVEINSTANCE = 0x02000000,

    rpGEOMETRYFLAGSMASK       = 0x000000FF,
    rpGEOMETRYNATIVEFLAGSMASK = 0x0F000000
};
RpMeshHeader *rpMeshRead( void *stream, const RpGeometry *geometry,
                          const RpMaterialList *matList ) noexcept
{
    binMeshHeader bmh{};
    RpMeshHeader *meshHeader;
    uint32_t      size;

    /* Read in a header */
    if ( !RwStreamRead( stream, &bmh, sizeof( bmh ) ) )
    {
        /* Failure */
        return nullptr;
    }

    /* Figure out the size (little bit of slack for alignment) */
    size = ( sizeof( RpMeshHeader ) +
             ( ( sizeof( RpMesh ) + 4 ) * bmh.numMeshes ) );

    if ( !( rpGEOMETRYNATIVE & geometry->flags ) )
    {
        size += ( sizeof( uint16_t ) * bmh.totalIndicesInMesh );
    }

    /* Allocate memory for the mesh */
    // RHDebug::DebugLogger::Log( "mesh sz:" + std::to_string( size ) );
    meshHeader = hAlloc<RpMeshHeader>( "MeshHeader", size );
    if ( !meshHeader )
        return nullptr;
    memset( meshHeader, 0, size );
    //_rpMeshHeaderCreate( size );
    if ( meshHeader )
    {
        RpMesh *  mesh = reinterpret_cast<RpMesh *>( meshHeader + 1 );
        uint16_t *meshIndices =
            reinterpret_cast<uint16_t *>( mesh + bmh.numMeshes );
        uint32_t numMeshes;

        meshHeader->flags     = bmh.flags;
        meshHeader->numMeshes = static_cast<uint16_t>( bmh.numMeshes );
        meshHeader->serialNum = static_cast<uint16_t>( g_nMeshSerialNum );
        meshHeader->totalIndicesInMesh =
            static_cast<uint32_t>( bmh.totalIndicesInMesh );
        meshHeader->firstMeshOffset = 0;

        g_nMeshSerialNum++;

        /* Then for each mesh... */
        numMeshes = meshHeader->numMeshes;
        while ( numMeshes-- > 0 )
        {
            binMesh bm;

            /* ... do another header... */
            if ( NULL == RwStreamRead( stream, &bm, sizeof( bm ) ) )
            {
                /* Failure */
                return nullptr;
            }

            mesh->numIndices = bm.numIndices;
            mesh->material   = matList->materials[bm.matIndex];
            mesh->indices    = meshIndices;

            if ( !( rpGEOMETRYNATIVE & geometry->flags ) )
            {
                uint32_t remainingIndices;

                /* ...and all the indices */
                remainingIndices = mesh->numIndices;
                while ( remainingIndices > 0 )
                {
                    RpIndexBuffer IndexBuffer;
                    int32_t *     source =
                        reinterpret_cast<int32_t *>( &IndexBuffer[0] );
                    uint32_t readIndices;

                    readIndices = ( remainingIndices < RWINDEXBUFFERSIZE )
                                      ? ( remainingIndices )
                                      : ( RWINDEXBUFFERSIZE );

                    if ( !RwStreamRead( stream, source,
                                        sizeof( int32_t ) * readIndices ) )
                    {
                        /* Failure */
                        return nullptr;
                    }

                    remainingIndices -= readIndices;
                    while ( readIndices-- > 0 )
                    {
                        *meshIndices++ = static_cast<int16_t>( *source++ );
                    }
                }
            }

            mesh++;
        }
    }

    /* All done */
    return ( meshHeader );
}

} // namespace rh::rw::engine
