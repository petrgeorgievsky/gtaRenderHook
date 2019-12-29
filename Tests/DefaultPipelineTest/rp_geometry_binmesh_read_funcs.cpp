#include "rp_geometry_binmesh_read_funcs.h"
#include "rp_geometry_funcs.h"
#include "rp_matlist_funcs.h"
#include <DebugUtils/DebugLogger.h>

uint32_t RH_RWAPI::g_nMeshSerialNum = 0;

#define RWINDEXBUFFERSIZE 256
typedef RwUInt32    RpIndexBuffer[RWINDEXBUFFERSIZE];

RpMeshHeader* RH_RWAPI::rpMeshRead( void* stream, const RpGeometry* geometry, const RpMaterialList* matList ) noexcept
{
    binMeshHeader   bmh;
    RpMeshHeader* meshHeader;
    RwUInt32        size;

    /* Read in a header */
    if( !RwStreamRead( stream, &bmh, sizeof( bmh ) ) )
    {
        /* Failure */
        return nullptr;
    }

    /* Figure out the size (little bit of slack for alignment) */
    size = ( sizeof( RpMeshHeader ) + ( ( sizeof( RpMesh ) + 4 ) * bmh.numMeshes ) );

    if( !( rpGEOMETRYNATIVE & geometry->flags ) )
    {
        size += ( sizeof( RxVertexIndex ) * bmh.totalIndicesInMesh );
    }

    /* Allocate memory for the mesh */
    //RHDebug::DebugLogger::Log( "mesh sz:" + std::to_string( size ) );
    meshHeader = (RpMeshHeader*)malloc( size );
    if( !meshHeader )
        return nullptr;
    memset( meshHeader, 0, size );
    //_rpMeshHeaderCreate( size );
    if( meshHeader )
    {
        RpMesh* mesh = (RpMesh*)( meshHeader + 1 );
        RxVertexIndex * meshIndices = (RxVertexIndex*)( mesh + bmh.numMeshes );
        RwUInt32        numMeshes;

        meshHeader->flags = bmh.flags;
        meshHeader->numMeshes = (RwUInt16)bmh.numMeshes;
        meshHeader->serialNum = g_nMeshSerialNum;
        meshHeader->totalIndicesInMesh = (RwUInt32)bmh.totalIndicesInMesh;
        meshHeader->firstMeshOffset = 0;

        g_nMeshSerialNum++;

        /* Then for each mesh... */
        numMeshes = meshHeader->numMeshes;
        while( numMeshes-- > 0 )
        {
            binMesh bm;

            /* ... do another header... */
            if( NULL == RwStreamRead( stream, & bm, sizeof( bm ) ) )
            {
                /* Failure */
                return nullptr;
            }

            mesh->numIndices = bm.numIndices;
            mesh->material = matList->materials[bm.matIndex];
            mesh->indices = meshIndices;

            if( !( rpGEOMETRYNATIVE & geometry->flags ) )
            {
                RwUInt32    remainingIndices;

                /* ...and all the indices */
                remainingIndices = mesh->numIndices;
                while( remainingIndices > 0 )
                {
                    RpIndexBuffer   IndexBuffer;
                    RwInt32* source = (RwInt32*)& IndexBuffer[0];
                    RwUInt32        readIndices;

                    readIndices = ( remainingIndices < RWINDEXBUFFERSIZE ) ?
                        ( remainingIndices ) : ( RWINDEXBUFFERSIZE );

                    if( !RwStreamRead( stream, source,
                                            sizeof( RwInt32 ) * readIndices ) )
                    {
                        /* Failure */
                        return nullptr;
                    }

                    remainingIndices -= readIndices;
                    while( readIndices-- > 0 )
                    {
                        *meshIndices++ = (RxVertexIndex)( *source++ );
                    }
                }
            }

            mesh++;
        }
    }

    /* All done */
    return ( meshHeader );
}

bool RH_RWAPI::readGeometryMesh( void* stream, RpGeometry* geometry ) noexcept
{
    if( !RwStreamFindChunk( stream, rwID_EXTENSION, nullptr, nullptr ) )
        return false;
    if( !RwStreamFindChunk( stream, rwID_BINMESHPLUGIN, nullptr, nullptr ) )
        return false;
    geometry->mesh = rpMeshRead( stream, geometry, &geometry->matList );
    return geometry->mesh != nullptr;
}