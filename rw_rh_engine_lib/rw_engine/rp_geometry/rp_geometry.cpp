#include "rp_geometry.h"
#include "../rp_matlist/rp_matlist.h"
#include "../rp_mesh/rp_mesh.h"
#include "../rw_macro_constexpr.h"
#include "../rw_stream/rw_stream.h"
#include <DebugUtils/DebugLogger.h>
#include <common_headers.h>
#include <cstdlib>
namespace rh::rw::engine {

rpGeometryList *GeometryListStreamRead( void *stream, rpGeometryList *geomList )
{
    RwInt32 gl;
    RwInt32 i;
    RwUInt32 size;
    RwUInt32 version;

    if ( !RwStreamFindChunk( stream, rwID_STRUCT, &size, &version ) ) {
        return nullptr;
    }

    /* Read it */
    if ( RwStreamRead( stream, &gl, sizeof( gl ) ) != sizeof( gl ) ) {
        return nullptr;
    }

    /* Set up the geometry list */
    geomList->numGeoms = 0;

    if ( gl > 0 ) {
        geomList->geometries = static_cast<RpGeometry **>(
            malloc( sizeof( RpGeometry * ) * static_cast<size_t>( gl ) ) );
        if ( !geomList->geometries ) {
            return nullptr;
        }
    } else {
        geomList->geometries = nullptr;
    }

    for ( i = 0; i < gl; i++ ) {
        /* Read the geometry */
        if ( !RwStreamFindChunk( stream, rwID_GEOMETRY, nullptr, &version ) ) {
            GeometryListDeinitialize( geomList );
            return nullptr;
        }

        if ( !( geomList->geometries[i] = RpGeometryStreamRead( stream ) ) ) {
            GeometryListDeinitialize( geomList );
            return nullptr;
        }
        /* Increment the number of geometries in the list */
        geomList->numGeoms++;
    }
    return geomList;
}

RpGeometry *RpGeometryStreamRead( void *stream )
{
    RpGeometry *geometry;
    _rpGeometry geom;
    RwUInt32 version;

    struct _rpGeometry_Rw_3_0_ext
    {
        float ambient;
        float diffuse;
        float specular;
    } geom_ext;

    if ( !RwStreamFindChunk( stream, rwID_STRUCT, nullptr, &version ) )
        return nullptr;

    if ( RwStreamRead( stream, &geom, sizeof( geom ) ) != sizeof( geom ) ) {
        return nullptr;
    }
    if ( version <= 0x00032000 || version == 0x00033002 ) {
        if ( RwStreamRead( stream, &geom_ext, sizeof( geom_ext ) ) != sizeof( geom_ext ) ) {
            return nullptr;
        }
    }

    geometry = RpGeometryCreate( geom.numVertices,
                                 geom.numTriangles,
                                 static_cast<uint32_t>( geom.format ) );

    if ( !geometry )
        return nullptr;

    if ( geom.numMorphTargets > 1 ) {
        if ( rh::rw::engine::RpGeometryAddMorphTargets( geometry, geom.numMorphTargets - 1 ) < 0 ) {
            rh::rw::engine::RpGeometryDestroy( geometry );
            return nullptr;
        }
    }

    if ( !( rpGEOMETRYNATIVE & geometry->flags ) ) {
        if ( geometry->numVertices ) {
            /* Read prelighting information */
            if ( geom.format & rpGEOMETRYPRELIT ) {
                RwUInt32 sizeLum = static_cast<uint32_t>( geometry->numVertices )
                                   * sizeof( RwRGBA );

                /* No conversion needed - it's made of chars */
                if ( RwStreamRead( stream, geometry->preLitLum, sizeLum ) != sizeLum ) {
                    /* Failed, so tidy up */
                    rh::rw::engine::RpGeometryDestroy( geometry );
                    return nullptr;
                }
            }

            /* Read texture coordinate information */
            if ( geometry->numTexCoordSets > 0 ) {
                RwInt32 i;
                const RwUInt32 sizeTC = static_cast<uint32_t>( geometry->numVertices )
                                        * sizeof( RwTexCoords );

                /* Read vertex texture coordinates - reals, remember */
                for ( i = 0; i < geometry->numTexCoordSets; i++ ) {
                    if ( !RwStreamRead( stream,
                                        reinterpret_cast<void *>( geometry->texCoords[i] ),
                                        sizeTC ) ) {
                        /* Failed, so tidy up */
                        rh::rw::engine::RpGeometryDestroy( geometry );
                        return nullptr;
                    }
                }
            }

            /* Read in the triangles */
            if ( geometry->numTriangles ) {
                RpTriangle *destTri;
                RwInt32 numTris;
                RwUInt32 size;

                /* Load the triangle information */
                numTris = geometry->numTriangles;
                destTri = geometry->triangles;

                size = static_cast<uint32_t>( numTris ) * sizeof( _rpTriangle );
                if ( RwStreamRead( stream, reinterpret_cast<void *>( destTri ), size ) != size
                     || destTri == nullptr ) {
                    rh::rw::engine::RpGeometryDestroy( geometry );

                    return nullptr;
                }

                /* These are unpacked "in-place" into RwUInt16 fields */
                // these guys smart tricksters tbh, why tho?
                while ( numTris-- ) {
                    RwUInt16 hi, lo;
                    _rpTriangle *srceTri;

                    srceTri = reinterpret_cast<_rpTriangle *>( destTri );

                    hi = static_cast<RwUInt16>( srceTri->vertex01 >> 16 ) & 0xFFFF;
                    lo = static_cast<RwUInt16>( srceTri->vertex01 ) & 0xFFFF;
                    destTri->vertIndex[0] = hi;
                    destTri->vertIndex[1] = lo;

                    hi = static_cast<RwUInt16>( srceTri->vertex2Mat >> 16 ) & 0xFFFF;
                    lo = static_cast<RwUInt16>( srceTri->vertex2Mat ) & 0xFFFF;
                    destTri->vertIndex[2] = hi;
                    destTri->matIndex = lo;

                    destTri++;
                }
            }
        }
    }

    RwInt32 i;

    for ( i = 0; i < geometry->numMorphTargets; i++ ) {
        struct _rpMorphTarget
        {
            RwSphere boundingSphere;
            RwBool pointsPresent;
            RwBool normalsPresent;
        } kf{};
        RpMorphTarget *morphTarget = &geometry->morphTarget[i];

        if ( RwStreamRead( stream, &kf, sizeof( kf ) ) != sizeof( kf ) ) {
            rh::rw::engine::RpGeometryDestroy( geometry );
            return nullptr;
        }
        morphTarget->boundingSphere = kf.boundingSphere;
        if ( kf.pointsPresent && kf.normalsPresent ) {
            if ( !RwStreamRead( stream,
                                morphTarget->verts,
                                sizeof( RwV3d ) * 2
                                    * static_cast<RwUInt32>( geometry->numVertices ) ) ) {
                rh::rw::engine::RpGeometryDestroy( geometry );
                return nullptr;
            }
        } else {
            if ( kf.pointsPresent ) {
                if ( !RwStreamRead( stream,
                                    morphTarget->verts,
                                    sizeof( RwV3d )
                                        * static_cast<RwUInt32>( geometry->numVertices ) ) ) {
                    rh::rw::engine::RpGeometryDestroy( geometry );
                    return nullptr;
                }
            }
            if ( kf.normalsPresent ) {
                if ( !RwStreamRead( stream,
                                    morphTarget->normals,
                                    sizeof( RwV3d )
                                        * static_cast<RwUInt32>( geometry->numVertices ) ) ) {
                    rh::rw::engine::RpGeometryDestroy( geometry );
                    return nullptr;
                }
            }
        }
    }

    if ( !RwStreamFindChunk( stream, rwID_MATLIST, nullptr, &version ) ) {
        rh::rw::engine::RpGeometryDestroy( geometry );
        return nullptr;
    }

    if ( !_rpMaterialListStreamRead( stream, geometry->matList ) ) {
        rh::rw::engine::RpGeometryDestroy( geometry );
        return nullptr;
    }

    readGeometryMesh( stream, geometry );

    return geometry;
}

constexpr uint32_t GeometryFormatGetNumTexCoordSets( uint32_t _fmt )
{
    return ( ( (_fmt) &0xff0000 ) ? ( ( (_fmt) &0xff0000 ) >> 16 )
                                  : ( ( (_fmt) &rpGEOMETRYTEXTURED2 )
                                          ? 2
                                          : ( ( (_fmt) &rpGEOMETRYTEXTURED ) ? 1 : 0 ) ) );
}

RpGeometry *RpGeometryCreate( int32_t numVerts, int32_t numTriangles, uint32_t format )
{
    using logger = rh::debug::DebugLogger;

    RpGeometry *geometry = nullptr;
    intptr_t goffset = 0;
    RwUInt32 gsize = 0;
    RwUInt32 numTexCoordSets = 0;
    RwUInt32 flags = 0;

    // input values check
    if ( ( numVerts < 0 ) || ( numVerts >= 65536 ) || ( numTriangles < 0 ) ) {
        if ( numVerts < 0 ) {
            logger::Error( "You cannot construct an RpGeometry with a negative number "
                           "of vertices." );
        } else if ( numVerts >= 65536 ) {
            logger::Error( "Vertex overflow(for 16bit index buffer at least)" );
        }
        if ( numTriangles < 0 ) {
            logger::Error( "You cannot construct an RpGeometry with a negative number "
                           "of triangles." );
        }
        return nullptr;
    }

    flags = format & rpGEOMETRYFLAGSMASK;
    gsize = sizeof( RpGeometry ); // geometryTKList.sizeOfStruct;
    numTexCoordSets = GeometryFormatGetNumTexCoordSets( format );

    flags = ( flags & static_cast<uint32_t>( ~( rpGEOMETRYTEXTURED | rpGEOMETRYTEXTURED2 ) ) )
            | ( ( numTexCoordSets == 1 ) ? rpGEOMETRYTEXTURED
                                         : ( ( numTexCoordSets > 1 ) ? rpGEOMETRYTEXTURED2 : 0 ) );

    if ( !( rpGEOMETRYNATIVE & format ) ) {
        if ( flags & static_cast<RwInt32>( rpGEOMETRYPRELIT ) ) {
            gsize += sizeof( RwRGBA ) * static_cast<size_t>( numVerts );
        }

        if ( numTexCoordSets > 0 ) {
            /* Include space for tex coords */
            gsize += sizeof( RwTexCoords ) * static_cast<size_t>( numVerts ) * numTexCoordSets;
        }

        gsize += sizeof( RpTriangle ) * static_cast<size_t>( numTriangles );
    }

    geometry = static_cast<RpGeometry *>( malloc( gsize ) );

    if ( !geometry )
        return nullptr;

    _rpMaterialListInitialize( geometry->matList );

    /* Allocate initial array of morph targets (0) */
    geometry->morphTarget = nullptr;

    /* Set up key frames */
    geometry->numMorphTargets = 0;

    /* Set up type */
    rwObject::Initialize( geometry, rpGEOMETRY, 0 );

    /* Set the instancing information */
    geometry->repEntry = nullptr;

    /* Nothing locked since last time */
    geometry->lockedSinceLastInst = 0;

    /* Initialise ref count */
    geometry->refCount = 1;

    /* Set up the device information -> its locked! */
    geometry->mesh = nullptr;

    /* Set up texture coords */
    geometry->numTexCoordSets = static_cast<RwInt32>( numTexCoordSets );
    memset( geometry->texCoords, 0, rwMAXTEXTURECOORDS * sizeof( RwTexCoords * ) );

    geometry->preLitLum = nullptr;

    /* Set up the triangles */
    geometry->triangles = nullptr;
    geometry->numTriangles = numTriangles;

    /* Set the geometries flags */
    geometry->flags = flags | ( rpGEOMETRYNATIVEFLAGSMASK & format );
    geometry->numVertices = numVerts;

    if ( !( rpGEOMETRYNATIVE & format ) ) {
        logger::Log( "Geometry offset " + std::to_string( reinterpret_cast<intptr_t>( geometry ) ) );
        /* step past structure to allocate arrays */
        goffset = reinterpret_cast<intptr_t>( geometry ) + sizeof( RpGeometry );
        logger::Log( "Prelit offset " + std::to_string( ( goffset ) ) );

        /* Create prelight values if necessary */
        if ( ( flags & rpGEOMETRYPRELIT ) && numVerts ) {
            geometry->preLitLum = reinterpret_cast<RwRGBA *>( goffset );
            goffset += sizeof( RwRGBA * ) * static_cast<size_t>( numVerts );
        }

        /* Create texture coordinates if necessary - in the right place */
        if ( numTexCoordSets > 0 && numVerts ) {
            RwUInt32 i;

            for ( i = 0; i < numTexCoordSets; i++ ) {
                geometry->texCoords[i] = reinterpret_cast<RwTexCoords *>( goffset );
                goffset += sizeof( RwTexCoords ) * static_cast<size_t>( numVerts );
            }
        }

        /* Set up the triangles */
        if ( numTriangles ) {
            geometry->triangles = reinterpret_cast<RpTriangle *>( goffset );
            goffset += sizeof( RpTriangle ) * static_cast<size_t>( numTriangles );

            /* Setup all of the materials */
            for ( intptr_t i = 0; i < numTriangles; i++ ) {
                geometry->triangles[i].matIndex = 0xFFFF;
            }
        }
    }

    /* Allocate one key frame, because geometry is useless without it */
    if ( rh::rw::engine::RpGeometryAddMorphTarget( geometry ) < 0 ) {
        _rpMaterialListDeinitialize( geometry->matList );
        free( geometry );
        return nullptr;
    }

    /* Initialize the plugin memory */
    // rwPluginRegistryInitObject( &geometryTKList, geometry );

    /* All done */
    return geometry;
}

int32_t RpGeometryAddMorphTargets( RpGeometry *geometry, int32_t mtcount ) noexcept
{
    RwUInt32 mtsize = 0, bytes = 0;
    RpMorphTarget *morphTarget = nullptr;
    RwV3d *vertexData = nullptr;

    if ( rpGEOMETRYNATIVE & geometry->flags ) {
        mtsize = sizeof( RpMorphTarget );
    } else {
        mtsize = sizeof( RpMorphTarget )
                 + sizeof( RwV3d ) * static_cast<size_t>( geometry->numVertices );

        if ( rpGEOMETRYNORMALS & geometry->flags ) {
            mtsize += sizeof( RwV3d ) * static_cast<size_t>( geometry->numVertices );
        }
    }

    bytes = mtsize
            * ( static_cast<size_t>( geometry->numMorphTargets ) + static_cast<size_t>( mtcount ) );

    /* Is it a realloc or a first time alloc? */
    if ( geometry->morphTarget ) {
        RwUInt8 *src = nullptr, *dst = nullptr;
        RwInt32 len = 0;

        morphTarget = static_cast<RpMorphTarget *>( realloc( geometry->morphTarget, bytes ) );
        if ( !morphTarget ) {
            /* Failed to allocate memory for the new morph target array */
            // RWERROR( ( E_RW_NOMEM, ( bytes ) ) );
            return ( -1 );
        }

        /* we want the MorphTarget structures at the beginning so we need to
open a gap for mtcount MorphTargets */
        src = reinterpret_cast<RwUInt8 *>( morphTarget )
              + ( mtsize * static_cast<size_t>( geometry->numMorphTargets ) ) - 1;
        dst = src + ( sizeof( RpMorphTarget ) * static_cast<size_t>( mtcount ) );
        len = static_cast<int32_t>( mtsize * static_cast<size_t>( geometry->numMorphTargets ) )
              - static_cast<int32_t>( sizeof( RpMorphTarget )
                                      * static_cast<size_t>( geometry->numMorphTargets ) );
        while ( len-- ) {
            *dst-- = *src--;
        }
    } else {
        morphTarget = static_cast<RpMorphTarget *>( malloc( bytes ) );
        if ( !morphTarget ) {
            /* Failed to allocate memory for the new morph target array */
            // RWERROR( ( E_RW_NOMEM, ( bytes ) ) );
            return ( -1 );
        }
    }

    /* Add the extra frames */
    geometry->numMorphTargets += mtcount;

    /* Memory allocation is successful, so install */
    geometry->morphTarget = morphTarget;

    /* setup ALL pointers */
    vertexData = reinterpret_cast<RwV3d *>(
        reinterpret_cast<RwUInt8 *>( morphTarget )
        + ( sizeof( RpMorphTarget ) * static_cast<size_t>( geometry->numMorphTargets ) ) );

    for ( RwInt32 i = 0; i < geometry->numMorphTargets; i++ ) {
        RpMorphTarget *aMorph = &geometry->morphTarget[i];

        aMorph->verts = nullptr;
        aMorph->normals = nullptr;

        if ( !( rpGEOMETRYNATIVE & ( geometry->flags ) ) ) {
            if ( geometry->numVertices ) {
                aMorph->verts = vertexData;
                vertexData += geometry->numVertices;

                if ( rpGEOMETRYNORMALS & ( geometry->flags ) ) {
                    aMorph->normals = vertexData;
                    vertexData += geometry->numVertices;
                }
            }
        }
    }

    /* just initialize new ones */
    for ( RwInt32 i = geometry->numMorphTargets - mtcount; i < geometry->numMorphTargets; i++ ) {
        RpMorphTarget *aMorph = &geometry->morphTarget[i];

        /* Initialize */
        aMorph->boundingSphere.center.x = 0.0F;
        aMorph->boundingSphere.center.y = 0.0F;
        aMorph->boundingSphere.center.z = 0.0F;
        aMorph->boundingSphere.radius = 0.0F;
        aMorph->parentGeom = geometry;
    }

    /* Done */
    return ( geometry->numMorphTargets - mtcount );
}

int32_t RpGeometryAddMorphTarget( RpGeometry *geometry ) noexcept
{
    return ( rh::rw::engine::RpGeometryAddMorphTargets( geometry, 1 ) );
}

rpGeometryList *GeometryListDeinitialize( rpGeometryList *geomList )

{
    RwInt32 i;

    /* remove the read reference to each geometry */
    for ( i = 0; i < geomList->numGeoms; i++ ) {
        rh::rw::engine::RpGeometryDestroy( geomList->geometries[i] );
    }

    if ( geomList->geometries ) {
        free( geomList->geometries );
        geomList->geometries = nullptr;
    }

    return ( geomList );
}

static RwBool GeometryAnnihilate( RpGeometry *geometry )
{
    /* Temporarily bump up reference count to avoid assertion failures */
    geometry->refCount++;

    /* Lock it so it can be destroyed */
    // RpGeometryLock( geometry, ( rpGEOMETRYLOCKALL ) );

    /* DeInitialize the plugin memory */
    // rwPluginRegistryDeInitObject( &geometryTKList, geometry );

    /* destroy morphTargets */
    if ( geometry->morphTarget ) {
        free( geometry->morphTarget );
        geometry->morphTarget = nullptr;
    }

    _rpMaterialListDeinitialize( geometry->matList );

    /* Reinstate reference count */
    --geometry->refCount;

    free( geometry );

    return ( TRUE );
}

int32_t RpGeometryDestroy( RpGeometry *geometry )
{
    RwBool result = TRUE;

    /* Do we  actually need to blow it away  ? */
    if ( ( geometry->refCount - 1 ) <= 0 ) {
        /* Blow away it's instance copy if it has one */
        if ( geometry->repEntry ) {
            if ( geometry->repEntry->destroyNotify )
                geometry->repEntry->destroyNotify( geometry->repEntry );
            if ( geometry->repEntry->ownerRef ) {
                *( geometry->repEntry->ownerRef ) = nullptr;
            }
            free( geometry->repEntry );
            // RwResourcesFreeResEntry( geometry->repEntry );
        }

        /* RWCRTCHECKMEMORY(); */
        /*
     * Don't decrement the refCount until after the resource
     * entry has been destroyed as we many need to call API
     * geometry functions.
     */
        --geometry->refCount;
        /* RWCRTCHECKMEMORY(); */

        result = GeometryAnnihilate( geometry );
    } else {
        /* RWCRTCHECKMEMORY(); */
        --geometry->refCount;
        /* RWCRTCHECKMEMORY(); */
    }

    return ( result );
}

RpGeometry *RpGeometryAddRef( RpGeometry *geometry )
{
    geometry->refCount++;

    return ( geometry );
}

} // namespace rw_rh_engine
