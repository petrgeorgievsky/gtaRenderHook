#include "rp_geometry_read_funcs.h"
#include "rp_geometry_funcs.h"
#include "rp_matlist_read_funcs.h"
#include "rp_geometry_binmesh_read_funcs.h"

RpGeometry* RH_RWAPI::_RpGeometryStreamRead( void* stream )
{
    RpGeometry* geometry;
    _rpGeometry         geom;
    RwUInt32            version;

    struct _rpGeometry_Rw_3_0_ext
    {
        float ambient;
        float diffuse;
        float specular;
    } geom_ext;

    if( !RwStreamFindChunk( stream, (RwUInt32)rwID_STRUCT, (RwUInt32*)NULL, &version ) )
    {
        return nullptr;
    }

    if( RwStreamRead( stream, &geom, sizeof( geom ) ) != sizeof( geom ) )
    {
        return nullptr;
    }
    if( version <= 0x00032000 || version == 0x00033002 )
    {
        if( RwStreamRead( stream, &geom_ext, sizeof( geom_ext ) ) != sizeof( geom_ext ) )
        {
            return nullptr;
        }
    }

    geometry = _RpGeometryCreate( geom.numVertices, geom.numTriangles, geom.format );

    if( !geometry )
        return nullptr;

    if( geom.numMorphTargets > 1 )
    {
        if( _RpGeometryAddMorphTargets( *geometry, geom.numMorphTargets - 1 ) < 0 )
        {
            _RpGeometryDestroy( geometry );
            return nullptr;
        }
    }

    if( !( rpGEOMETRYNATIVE & geometry->flags ) )
    {
        if( geometry->numVertices )
        {
            /* Read prelighting information */
            if( geom.format & rpGEOMETRYPRELIT )
            {
                RwUInt32    sizeLum = geometry->numVertices * sizeof( RwRGBA );

                /* No conversion needed - it's made of chars */
                if( RwStreamRead( stream, geometry->preLitLum, sizeLum )
                    != sizeLum )
                {
                    /* Failed, so tidy up */
                    _RpGeometryDestroy( geometry );
                    return nullptr;
                }
            }

            /* Read texture coordinate information */
            if( geometry->numTexCoordSets > 0 )
            {
                RwInt32         i;
                const RwUInt32  sizeTC = geometry->numVertices *
                    sizeof( RwTexCoords );

                /* Read vertex texture coordinates - reals, remember */
                for( i = 0; i < geometry->numTexCoordSets; i++ )
                {
                    if( !RwStreamRead( stream, (void*)geometry->texCoords[i], sizeTC ) )
                    {
                        /* Failed, so tidy up */
                        _RpGeometryDestroy( geometry );
                        return nullptr;
                    }
                }
            }

            /* Read in the triangles */
            if( geometry->numTriangles )
            {
                RpTriangle* destTri;
                RwInt32     numTris;
                RwUInt32    size;

                /* Load the triangle information */
                numTris = geometry->numTriangles;
                destTri = geometry->triangles;

                size = numTris * sizeof( _rpTriangle );
                if( RwStreamRead( stream, (void*)destTri, size ) != size )
                {
                    _RpGeometryDestroy( geometry );

                    return nullptr;
                }

                /* These are unpacked "in-place" into RwUInt16 fields */
                // these guys smart tricksters tbh, why tho?
                while( numTris-- )
                {
                    RwUInt16    hi, lo;
                    _rpTriangle* srceTri;

                    srceTri = (_rpTriangle*)destTri;

                    hi = (RwUInt16)( srceTri->vertex01 >> 16 ) & 0xFFFF;
                    lo = (RwUInt16)( srceTri->vertex01 ) & 0xFFFF;
                    destTri->vertIndex[0] = hi;
                    destTri->vertIndex[1] = lo;

                    hi = (RwUInt16)( srceTri->vertex2Mat >> 16 ) & 0xFFFF;
                    lo = (RwUInt16)( srceTri->vertex2Mat ) & 0xFFFF;
                    destTri->vertIndex[2] = hi;
                    destTri->matIndex = lo;

                    destTri++;
                }
            }
        }
    }

    RwInt32             i;

    for( i = 0; i < geometry->numMorphTargets; i++ )
    {
        _rpMorphTarget      kf;
        RpMorphTarget* morphTarget = &geometry->morphTarget[i];

        if( RwStreamRead( stream, &kf, sizeof( kf ) ) != sizeof( kf ) )
        {
            _RpGeometryDestroy( geometry );
            return nullptr;
        }
        morphTarget->boundingSphere = kf.boundingSphere;
        if( kf.pointsPresent && kf.normalsPresent )
        {
            if( !RwStreamRead( stream, morphTarget->verts,
                               sizeof( RwV3d ) * 2 * geometry->numVertices ) )
            {
                _RpGeometryDestroy( geometry );
                return nullptr;
            }
        }
        else {
            if( kf.pointsPresent )
            {
                if( !RwStreamRead( stream, morphTarget->verts,
                                   sizeof( RwV3d ) * geometry->numVertices ) )
                {
                    _RpGeometryDestroy( geometry );
                    return nullptr;
                }
            }
            if( kf.normalsPresent )
            {
                if( !RwStreamRead( stream, morphTarget->normals,
                                   sizeof( RwV3d ) * geometry->numVertices ) )
                {
                    _RpGeometryDestroy( geometry );
                    return nullptr;
                }
            }
        }
    }

    if( !RwStreamFindChunk( stream, (RwUInt32)rwID_MATLIST,
        (RwUInt32*)NULL, &version ) )
    {
        _RpGeometryDestroy( geometry );
        return nullptr;
    }

    if( !_rpMaterialListStreamRead( stream, geometry->matList ) )
    {
        _RpGeometryDestroy( geometry );
        return nullptr;
    }

    readGeometryMesh( stream, geometry );

    return geometry;
}