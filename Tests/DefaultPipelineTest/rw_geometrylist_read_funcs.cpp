#include "rw_geometrylist_read_funcs.h"
#include "rp_geometry_read_funcs.h"

RH_RWAPI::rpGeometryList* RH_RWAPI::_GeometryListStreamRead( void* stream, rpGeometryList* geomList )
{
    RwInt32             gl;
    RwInt32             i;
    RwUInt32            size;
    RwUInt32            version;

    if( !RwStreamFindChunk( stream, rwID_STRUCT, &size, &version ) )
    {
        return nullptr;
    }

    /* Read it */
    if( RwStreamRead( stream, &gl, sizeof( gl ) ) != sizeof( gl ) )
    {
        return nullptr;
    }

    /* Set up the geometry list */
    geomList->numGeoms = 0;

    if( gl > 0 )
    {
        geomList->geometries = (RpGeometry * *)malloc( sizeof( RpGeometry* ) * gl );
        if( !geomList->geometries )
        {
            return nullptr;
        }
    }
    else
    {
        geomList->geometries = (RpGeometry * *)NULL;
    }

    for( i = 0; i < gl; i++ )
    {
        /* Read the geometry */
        if( !RwStreamFindChunk( stream, rwID_GEOMETRY,
            (RwUInt32*)NULL, &version ) )
        {
            //GeometryListDeinitialize( geomList );
            return nullptr;
        }

        if( !( geomList->geometries[i] = _RpGeometryStreamRead( stream ) ) )
        {
            //GeometryListDeinitialize( geomList );
            return nullptr;
        }
        /* Increment the number of geometries in the list */
        geomList->numGeoms++;
    }
    return geomList;
}