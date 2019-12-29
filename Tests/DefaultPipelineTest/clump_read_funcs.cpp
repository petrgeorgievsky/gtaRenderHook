#include "clump_read_funcs.h"
#include <DebugUtils/DebugLogger.h>
#include "rp_clump_funcs.h"
#include "rw_framelist_read_funcs.h"
#include "rw_geometrylist_read_funcs.h"
#include "rp_geometry_read_funcs.h"
#include "rp_geometry_funcs.h"
#include "rp_atomic_funcs.h"

using namespace std;
using namespace std::experimental::filesystem;
namespace RH_RWAPI
{

typedef struct rpAtomicBinary _rpAtomicBinary;
struct rpAtomicBinary
{
    RwInt32             frameIndex;
    RwInt32             geomIndex;
    RwInt32             flags;
    RwInt32             unused;
};

static RpAtomic* _ClumpAtomicStreamRead( void* stream, rwFrameList* fl, rpGeometryList* gl )
{
    RwBool              status;
    RwUInt32            size;
    RwUInt32            version;

    status = RwStreamFindChunk( stream, rwID_STRUCT, &size, &version );

    if( !status )
    {
        //RWERROR( ( E_RW_READ ) );
        return nullptr;
    }

    RpAtomic* atom;
    _rpAtomicBinary     a;
    RpGeometry* geom;

    /* Read the atomic */
    memset( &a, 0, sizeof( a ) );

    status = ( size == RwStreamRead( stream, &a, size ) );
    if( !status )
    {
        //RWERROR( ( E_RW_READ ) );
        return nullptr;
    }

    atom = _RpAtomicCreate();
    if( !atom )
    {
        return nullptr;
    }

    /* Set the atomic types */
    rwObjectSetFlags( atom, a.flags );
    if( fl->numFrames )
    {
        _RpAtomicSetFrame( atom, fl->frames[a.frameIndex] );
    }

    /* get the geometry */
    if( gl->numGeoms )
    {
        _RpAtomicSetGeometry( atom, gl->geometries[a.geomIndex], 0 );
    }
    else
    {
        status = RwStreamFindChunk( stream, rwID_GEOMETRY,
            (RwUInt32*)NULL, &version );
        if( !status )
        {
            //RpAtomicDestroy( atom );
            //RWERROR( ( E_RW_READ ) );
            return nullptr;
        }

        geom = _RpGeometryStreamRead( stream );
        status = ( NULL != geom );

        if( !status )
        {
            //RpAtomicDestroy( atom );
            //RWERROR( ( E_RW_READ ) );
            return nullptr;
        }

        _RpAtomicSetGeometry( atom, geom, 0 );

        /* Bring the geometry reference count back down, so that
            * when the atomic is destroyed, so is the geometry.
            */
        _RpGeometryDestroy( geom );
    }

    return ( atom );
}

static rpGeometryList* GeometryListDeinitialize( rpGeometryList* geomList )
{
    RwInt32             i;

    /* remove the read reference to each geometry */
    for( i = 0; i < geomList->numGeoms; i++ )
    {
        _RpGeometryDestroy( geomList->geometries[i] );
    }

    if( geomList->geometries )
    {
        free( geomList->geometries );
        geomList->geometries = (RpGeometry * *)NULL;
    }

    return ( geomList );
}

RpClump* _RpClumpStreamRead( void* stream )
{
    using logger = RHDebug::DebugLogger;
    RwBool              status;
    RwUInt32            size;
    RwUInt32            version;

    status = RwStreamFindChunk( stream, rwID_STRUCT, &size, &version );

    if( !status )
    {
        return nullptr;
    }

    RwUInt32            chunkversion;
    _rpClump            cl{};
    rwFrameList         fl{};
    rpGeometryList      gl{};
    RpAtomic* atom;

    logger::Log( "RpClumpStreamRead: reading _rpClump info:\n" );

    if( version > 0x33000 ) {
        status = ( sizeof( cl ) == RwStreamRead( stream, &cl, sizeof( cl ) ) );

        logger::Log( "numAtomics:" +
                     std::to_string( cl.numAtomics ) +
                     "\tnumCameras:" +
                     std::to_string( cl.numCameras ) +
                     "\tnumLights:" +
                     std::to_string( cl.numLights )
        );
    }
    else {
        status = ( sizeof( cl.numAtomics ) == RwStreamRead( stream, &cl.numAtomics, sizeof( cl.numAtomics ) ) );

        logger::Log( "numAtomics:" +
                     std::to_string( cl.numAtomics ) );
    }

    RpClump * clump = _RpClumpCreate();

    status = RwStreamFindChunk( stream, (RwUInt32)rwID_FRAMELIST,
        (RwUInt32*)NULL, &chunkversion );

    status = ( NULL != _rwFrameListStreamRead( stream, &fl ) );

    rwObjectSetParent( clump, fl.frames[0] );

    status = RwStreamFindChunk( stream, (RwUInt32)rwID_GEOMETRYLIST, (RwUInt32*)NULL, &chunkversion );

    status = ( NULL != _GeometryListStreamRead( stream, &gl ) );

    if( !status )
    {
        if( fl.numFrames )
            free( fl.frames );
        _RpClumpDestroy( clump );
        logger::Error( "RpClumpStreamRead: failed to read geometry list!" );
        return nullptr;
    }

    /* Iterate over the atomics */
    for( uint32_t i = 0; i < static_cast<uint32_t> (cl.numAtomics); i++ )
    {
        status = RwStreamFindChunk( stream, (RwUInt32)rwID_ATOMIC,
            (RwUInt32*)NULL, &version );
        if( status )
        {
            atom = _ClumpAtomicStreamRead( stream, &fl, &gl );
            status = ( NULL != atom );
        }
        else
        {
            GeometryListDeinitialize( &gl );
            if( fl.numFrames )
                free( fl.frames );
            _RpClumpDestroy( clump );
            logger::Error( "RpClumpStreamRead: failed to read atomic!" );
            return nullptr;
        }

        /* Add the atomic to the clump */
        _RpClumpAddAtomic( clump, atom );
    }

    /* Dont need the geometry list anymore */
    GeometryListDeinitialize( &gl );

    /* Dont need the frame list anymore */
    if( fl.numFrames )
        free( fl.frames );
    

    return clump;
}

bool LoadClump( RpClump* &clump, const path& dff_path )
{
    RHDebug::DebugLogger::Log( "LoadClump begin " + dff_path.filename().generic_string() );
    ifstream stream( dff_path, ios_base::in | ios_base::binary );

    if( RH_RWAPI::RwStreamFindChunk( &stream, rwID_CLUMP, nullptr, nullptr ) )
        clump = _RpClumpStreamRead( &stream );
    
    return clump != nullptr;
}

}