#include "rp_geometry_funcs.h"
#include <DebugUtils/DebugLogger.h>
#include "rp_matlist_funcs.h"

constexpr RwInt32 RH_RWAPI::GeometryFormatGetNumTexCoordSets( RwInt32 _fmt )
{
    return ( ( ( _fmt ) & 0xff0000 ) ? ( ( ( _fmt ) & 0xff0000 ) >> 16 ) :
        ( ( (_fmt)& rpGEOMETRYTEXTURED2 ) ? 2 :
             ( ( (_fmt)& rpGEOMETRYTEXTURED ) ? 1 : 0 ) ) );
}

RpGeometry* RH_RWAPI::_RpGeometryCreate( RwInt32 numVerts, RwInt32 numTriangles, RwUInt32 format )
{
    using logger = RHDebug::DebugLogger;

    RpGeometry* geometry = nullptr;
    RwUInt8* goffset = nullptr;
    RwUInt32        gsize=0;
    RwUInt32        numTexCoordSets=0;
    RwInt32         flags=0;

    // input values check
    if( ( numVerts < 0 ) || ( numVerts >= 65536 ) || ( numTriangles < 0 ) )
    {
        if( numVerts < 0 )
        {
            logger::Error( "You cannot construct an RpGeometry with a negative number of vertices." );
        }
        else if( numVerts >= 65536 )
        {
            logger::Error( "Vertex overflow(for 16bit index buffer at least)" );
        }
        if( numTriangles < 0 )
        {
            logger::Error( "You cannot construct an RpGeometry with a negative number of triangles." );
        }
        return nullptr;
    }

    flags = format & rpGEOMETRYFLAGSMASK;
    gsize = sizeof( RpGeometry );//geometryTKList.sizeOfStruct;
    numTexCoordSets = GeometryFormatGetNumTexCoordSets( format );

    flags = ( flags & ~( rpGEOMETRYTEXTURED | rpGEOMETRYTEXTURED2 ) )
        | ( ( numTexCoordSets == 1 ) ? rpGEOMETRYTEXTURED :
        ( ( numTexCoordSets > 1 ) ? rpGEOMETRYTEXTURED2 : 0 ) );

    if( !( rpGEOMETRYNATIVE& format ) )
    {
        if( flags & static_cast<RwInt32> ( rpGEOMETRYPRELIT ) )
        {
            gsize += sizeof( RwRGBA ) * numVerts;
        }

        if( numTexCoordSets > 0 )
        {
            /* Include space for tex coords */
            gsize += sizeof( RwTexCoords ) * numVerts * numTexCoordSets;
        }

        gsize += sizeof( RpTriangle ) * numTriangles;
    }

    geometry = (RpGeometry*)malloc( gsize );

    if( !geometry )
        return nullptr;

    _rpMaterialListInitialize( geometry->matList );

    /* Allocate initial array of morph targets (0) */
    geometry->morphTarget = nullptr;

    /* Set up key frames */
    geometry->numMorphTargets = 0;

    /* Set up type */
    rwObjectInitialize( geometry, rpGEOMETRY, 0 );

    /* Set the instancing information */
    geometry->repEntry = nullptr;

    /* Nothing locked since last time */
    geometry->lockedSinceLastInst = 0;

    /* Initialise ref count */
    geometry->refCount = 1;

    /* Set up the device information -> its locked! */
    geometry->mesh = nullptr;

    /* Set up texture coords */
    geometry->numTexCoordSets = numTexCoordSets;
    memset( geometry->texCoords, 0, rwMAXTEXTURECOORDS * sizeof( RwTexCoords* ) );

    geometry->preLitLum = nullptr;

    /* Set up the triangles */
    geometry->triangles = nullptr;
    geometry->numTriangles = numTriangles;

    /* Set the geometries flags */
    geometry->flags = flags | ( rpGEOMETRYNATIVEFLAGSMASK & format );
    geometry->numVertices = numVerts;

    if( !( rpGEOMETRYNATIVE & format ) )
    {
        /* step past structure to allocate arrays */
        goffset = (RwUInt8*)geometry + sizeof( RpGeometry );

        /* Create prelight values if necessary */
        if( ( flags & (RwInt32)rpGEOMETRYPRELIT ) && numVerts )
        {
            geometry->preLitLum = (RwRGBA*)goffset;
            goffset += sizeof( RwRGBA* ) * numVerts;
        }

        /* Create texture coordinates if necessary - in the right place */
        if( numTexCoordSets > 0 && numVerts )
        {
            RwUInt32    i;

            for( i = 0; i < numTexCoordSets; i++ )
            {
                geometry->texCoords[i] = (RwTexCoords*)goffset;
                goffset += sizeof( RwTexCoords ) * numVerts;
            }
        }

        /* Set up the triangles */
        if( numTriangles )
        {
            geometry->triangles = (RpTriangle*)goffset;
            goffset += sizeof( RpTriangle ) * numTriangles;

            /* Setup all of the materials */
            for( RwInt32 i = 0; i < numTriangles; i++ )
            {
                geometry->triangles[i].matIndex = 0xFFFF;
            }
        }
    }

    /* Allocate one key frame, because geometry is useless without it */
    if( _RpGeometryAddMorphTarget( *geometry ) < 0 )
    {
        _rpMaterialListDeinitialize( geometry->matList );
        free( geometry );
        return nullptr;
    }

    /* Initialize the plugin memory */
    //rwPluginRegistryInitObject( &geometryTKList, geometry );

    /* All done */
    return geometry;
}

RwBool RH_RWAPI::_GeometryAnnihilate( RpGeometry* geometry )
{
    /* Temporarily bump up reference count to avoid assertion failures */
    geometry->refCount++;

    /* Lock it so it can be destroyed */
    //RpGeometryLock( geometry, ( rpGEOMETRYLOCKALL ) );

    /* DeInitialize the plugin memory */
    //rwPluginRegistryDeInitObject( &geometryTKList, geometry );

    /* destroy morphTargets */
    if( geometry->morphTarget )
    {
        free( geometry->morphTarget );
        geometry->morphTarget = nullptr;
    }

    _rpMaterialListDeinitialize( geometry->matList );

    /* Reinstate reference count */
    --geometry->refCount;

    free( geometry );

    return ( TRUE );
}

RwBool RH_RWAPI::_RpGeometryDestroy( RpGeometry* geometry )

{
    RwBool              result = TRUE;

    /* Do we  actually need to blow it away  ? */
    if( ( geometry->refCount - 1 ) <= 0 )
    {
        /* Blow away it's instance copy if it has one */
        if( geometry->repEntry )
        {
            if( geometry->repEntry->destroyNotify )
                geometry->repEntry->destroyNotify( geometry->repEntry );
            if( geometry->repEntry->ownerRef )
            {
                *( geometry->repEntry->ownerRef ) = (RwResEntry*)NULL;
            }
            free( geometry->repEntry );
            //RwResourcesFreeResEntry( geometry->repEntry );
        }

        /* RWCRTCHECKMEMORY(); */
        /*
         * Don't decrement the refCount until after the resource
         * entry has been destroyed as we many need to call API
         * geometry functions.
         */
        --geometry->refCount;
        /* RWCRTCHECKMEMORY(); */

        result = _GeometryAnnihilate( geometry );
    }
    else
    {
        /* RWCRTCHECKMEMORY(); */
        --geometry->refCount;
        /* RWCRTCHECKMEMORY(); */
    }

    return ( result );
}

RwInt32 RH_RWAPI::_RpGeometryAddMorphTargets( RpGeometry& geometry, RwInt32 mtcount ) noexcept
{
    RwUInt32            mtsize = 0, bytes = 0;
    RpMorphTarget* morphTarget = nullptr;
    RwV3d* vertexData = nullptr;

    if( rpGEOMETRYNATIVE& geometry.flags )
    {
        mtsize = sizeof( RpMorphTarget );
    }
    else
    {
        mtsize = sizeof( RpMorphTarget ) + sizeof( RwV3d ) * geometry.numVertices;

        if( rpGEOMETRYNORMALS& geometry.flags )
        {
            mtsize += sizeof( RwV3d ) * geometry.numVertices;
        }
    }

    bytes = mtsize * ( geometry.numMorphTargets + mtcount );

    /* Is it a realloc or a first time alloc? */
    if( geometry.morphTarget )
    {
        RwUInt8* src = nullptr, * dst = nullptr;
        RwInt32 len = 0;

        morphTarget = (RpMorphTarget*)realloc( geometry.morphTarget, bytes );
        if( !morphTarget )
        {
            /* Failed to allocate memory for the new morph target array */
           // RWERROR( ( E_RW_NOMEM, ( bytes ) ) );
            return ( -1 );
        }

        /* we want the MorphTarget structures at the beginning so we need to
           open a gap for mtcount MorphTargets */
        src = (RwUInt8*)morphTarget + ( mtsize * geometry.numMorphTargets ) - 1;
        dst = src + ( sizeof( RpMorphTarget ) * mtcount );
        len = ( mtsize * geometry.numMorphTargets ) - ( sizeof( RpMorphTarget ) * geometry.numMorphTargets );
        while( len-- )
        {
            *dst-- = *src--;
        }
    }
    else
    {
        morphTarget = (RpMorphTarget*)malloc( bytes );
        if( !morphTarget )
        {
            /* Failed to allocate memory for the new morph target array */
            //RWERROR( ( E_RW_NOMEM, ( bytes ) ) );
            return ( -1 );
        }
    }

    /* Add the extra frames */
    geometry.numMorphTargets += mtcount;

    /* Memory allocation is successful, so install */
    geometry.morphTarget = morphTarget;

    /* setup ALL pointers */
    vertexData = (RwV3d*)( (RwUInt8*)morphTarget +
        ( sizeof( RpMorphTarget ) *
          geometry.numMorphTargets ) );

    for( RwInt32 i = 0; i < geometry.numMorphTargets; i++ )
    {
        RpMorphTarget* aMorph = &geometry.morphTarget[i];

        aMorph->verts = nullptr;
        aMorph->normals = nullptr;

        if( !( rpGEOMETRYNATIVE & ( geometry.flags ) ) )
        {
            if( geometry.numVertices )
            {
                aMorph->verts = vertexData;
                vertexData += geometry.numVertices;

                if( rpGEOMETRYNORMALS & ( geometry.flags ) )
                {
                    aMorph->normals = vertexData;
                    vertexData += geometry.numVertices;
                }
            }
        }
    }

    /* just initialize new ones */
    for( RwInt32 i = geometry.numMorphTargets - mtcount; i < geometry.numMorphTargets; i++ )
    {
        RpMorphTarget* aMorph = &geometry.morphTarget[i];

        /* Initialize */
        aMorph->boundingSphere.center.x = (RwReal)( 0 );
        aMorph->boundingSphere.center.y = (RwReal)( 0 );
        aMorph->boundingSphere.center.z = (RwReal)( 0 );
        aMorph->boundingSphere.radius = (RwReal)( 0 );
        aMorph->parentGeom = &geometry;
    }

    /* Done */
    return ( geometry.numMorphTargets - mtcount );
}

RwInt32 RH_RWAPI::_RpGeometryAddMorphTarget( RpGeometry& geometry ) noexcept
{
    return ( _RpGeometryAddMorphTargets( geometry, 1 ) );
}

RpGeometry* RH_RWAPI::_RpGeometryAddRef( RpGeometry* geometry )
{
    geometry->refCount++;

    return ( geometry );
}
