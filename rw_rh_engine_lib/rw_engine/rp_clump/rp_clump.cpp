#include "rp_clump.h"
#include "../rp_atomic/rp_atomic.h"
#include "../rp_geometry/rp_geometry.h"
#include "../rw_frame/rw_frame.h"
#include "../rw_macro_constexpr.h"
#include "../rw_stream/rw_stream.h"
#include <DebugUtils/DebugLogger.h>
#include <common_headers.h>
#include <fstream>

bool rh::rw::engine::LoadClump( RpClump *&                   clump,
                                const std::filesystem::path &dff_path )
{
    std::ifstream stream( dff_path, std::ios_base::in | std::ios_base::binary );

    if ( RwStreamFindChunk( &stream, rwID_CLUMP, nullptr, nullptr ) )
        clump = RpClumpStreamRead( &stream );

    return clump != nullptr;
}

RpClump *rh::rw::engine::RpClumpStreamRead( void *stream )
{
    using logger = rh::debug::DebugLogger;
    RwBool   status;
    RwUInt32 size;
    RwUInt32 version;

    status = RwStreamFindChunk( stream, rwID_STRUCT, &size, &version );

    if ( !status )
    {
        return nullptr;
    }

    RwUInt32       chunkversion;
    _rpClump       cl{};
    rwFrameList    fl{};
    rpGeometryList gl{};
    RpAtomic *     atom;

    logger::Log( "RpClumpStreamRead: reading _rpClump info:\n" );

    if ( version > 0x33000 )
    {
        status = ( sizeof( cl ) == RwStreamRead( stream, &cl, sizeof( cl ) ) );

        logger::Log( "numAtomics:" + std::to_string( cl.numAtomics ) +
                     "\tnumCameras:" + std::to_string( cl.numCameras ) +
                     "\tnumLights:" + std::to_string( cl.numLights ) );
    }
    else
    {
        status =
            ( sizeof( cl.numAtomics ) ==
              RwStreamRead( stream, &cl.numAtomics, sizeof( cl.numAtomics ) ) );

        logger::Log( "numAtomics:" + std::to_string( cl.numAtomics ) );
    }

    RpClump *clump = RpClumpCreate();

    status =
        RwStreamFindChunk( stream, rwID_FRAMELIST, nullptr, &chunkversion );

    status = ( nullptr != _rwFrameListStreamRead( stream, &fl ) );

    rwObject::SetParent( clump, fl.frames[0] );

    status =
        RwStreamFindChunk( stream, rwID_GEOMETRYLIST, nullptr, &chunkversion );

    status = ( nullptr != GeometryListStreamRead( stream, &gl ) );

    if ( !status )
    {
        if ( fl.numFrames )
            free( fl.frames );
        rh::rw::engine::RpClumpDestroy( clump );
        logger::Error( "RpClumpStreamRead: failed to read geometry list!" );
        return nullptr;
    }

    /* Iterate over the atomics */
    for ( uint32_t i = 0; i < static_cast<uint32_t>( cl.numAtomics ); i++ )
    {
        status = RwStreamFindChunk( stream, rwID_ATOMIC, nullptr, &version );
        if ( status )
        {
            atom   = ClumpAtomicStreamRead( stream, &fl, &gl );
            status = ( nullptr != atom );
        }
        else
        {
            GeometryListDeinitialize( &gl );
            if ( fl.numFrames )
                free( fl.frames );
            rh::rw::engine::RpClumpDestroy( clump );
            logger::Error( "RpClumpStreamRead: failed to read atomic!" );
            return nullptr;
        }

        /* Add the atomic to the clump */
        rh::rw::engine::RpClumpAddAtomic( clump, atom );
    }

    /* Dont need the geometry list anymore */
    GeometryListDeinitialize( &gl );

    /* Dont need the frame list anymore */
    if ( fl.numFrames )
        free( fl.frames );

    return clump;
}

RpClump *rh::rw::engine::RpClumpCreate() noexcept
{
    RpClump *clump;

    clump = static_cast<RpClump *>( malloc( sizeof( RpClump ) ) );

    if ( !clump )
    {
        return ( clump );
    }

    rwObject::Initialize( clump, rpCLUMP, 0 );
    //_RpClumpSetFrame( clump, NULL );

    /* Contains nothing */
    rwLinkList::Initialize( &clump->atomicList );
    rwLinkList::Initialize( &clump->lightList );
    rwLinkList::Initialize( &clump->cameraList );

    /* Its not in the world */
    rwLLLink::Initialize( &clump->inWorldLink );

    /* Set the callback */
    // RpClumpSetCallBack( clump, (RpClumpCallBack)NULL );

    /* Initialize memory allocated to toolkits */
    // rwPluginRegistryInitObject( &clumpTKList, clump );

    /* All Done */
    return ( clump );
}

int32_t rh::rw::engine::RpClumpDestroy( RpClump *clump )
{
    // RwFrame *frame = nullptr;

    /* De-init the clump plugin registered memory */
    // rwPluginRegistryDeInitObject( &clumpTKList, clump );

    // RpClumpForAllAtomics( clump, DestroyClumpAtomic, NULL );
    // RpClumpForAllLights( clump, DestroyClumpLight, NULL );
    // RpClumpForAllCameras( clump, DestroyClumpCamera, NULL );

    /* Destroy the frame hierarchy if one exists */
    // frame = RpClumpGetFrame( clump );
    /*if( frame )
{
RwFrameDestroyHierarchy( frame );
}*/

    /* Destroy the clump */
    free( clump );

    return ( TRUE );
}

RpClump *rh::rw::engine::RpClumpAddAtomic( RpClump * clump,
                                           RpAtomic *atomic ) noexcept
{
    /* It is assumed the atomic is NOT in the world although the
     * clump might be
     */

    rwLinkList::AddLLLink( &clump->atomicList, &atomic->inClumpLink );
    atomic->clump = clump;

    return ( clump );
}

struct rpAtomicBinary
{
    RwInt32 frameIndex;
    RwInt32 geomIndex;
    RwInt32 flags;
    RwInt32 unused;
};

RpAtomic *rh::rw::engine::ClumpAtomicStreamRead( void *stream, rwFrameList *fl,
                                                 rpGeometryList *gl )
{
    RwBool   status;
    RwUInt32 size;
    RwUInt32 version;

    status = RwStreamFindChunk( stream, rwID_STRUCT, &size, &version );

    if ( !status )
    {
        // RWERROR( ( E_RW_READ ) );
        return nullptr;
    }

    RpAtomic *     atom;
    rpAtomicBinary a{};
    RpGeometry *   geom;

    /* Read the atomic */
    memset( &a, 0, sizeof( a ) );

    status = ( size == RwStreamRead( stream, &a, size ) );
    if ( !status )
    {
        // RWERROR( ( E_RW_READ ) );
        return nullptr;
    }

    atom = RpAtomicCreate();
    if ( !atom )
    {
        return nullptr;
    }

    /* Set the atomic types */
    rwObject::SetFlags( atom, static_cast<uint8_t>( a.flags ) );
    if ( fl->numFrames )
    {
        rh::rw::engine::RpAtomicSetFrame( atom, fl->frames[a.frameIndex] );
    }

    /* get the geometry */
    if ( gl->numGeoms )
    {
        rh::rw::engine::RpAtomicSetGeometry( atom, gl->geometries[a.geomIndex],
                                             0 );
    }
    else
    {
        status = RwStreamFindChunk( stream, rwID_GEOMETRY, nullptr, &version );
        if ( !status )
        {
            rh::rw::engine::RpAtomicDestroy( atom );
            // RWERROR( ( E_RW_READ ) );
            return nullptr;
        }

        geom   = RpGeometryStreamRead( stream );
        status = ( nullptr != geom );

        if ( !status )
        {
            rh::rw::engine::RpAtomicDestroy( atom );
            // RWERROR( ( E_RW_READ ) );
            return nullptr;
        }

        rh::rw::engine::RpAtomicSetGeometry( atom, geom, 0 );

        /* Bring the geometry reference count back down, so that
         * when the atomic is destroyed, so is the geometry.
         */
        rh::rw::engine::RpGeometryDestroy( geom );
    }

    return ( atom );
}
