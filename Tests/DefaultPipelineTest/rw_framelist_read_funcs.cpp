#include "rw_framelist_read_funcs.h"
#include "rw_frame_funcs.h"
#include <DebugUtils/DebugLogger.h>

std::string RH_RWAPI::RwV3dToStr( RwV3d vec ) 
{
    return
        "x:" + std::to_string( vec.x ) + "\t"
        "y:" + std::to_string( vec.y ) + "\t"
        "z:" + std::to_string( vec.z );
}

rwFrameList* RH_RWAPI::_rwFrameListStreamRead( void* stream, rwFrameList* frameList )
{
    using logger = RHDebug::DebugLogger;

    logger::Log( "rwFrameListStreamRead start" );

    struct rwStreamFrameList
    {
        RwInt32             numFrames;
    } fl;
    RwInt32             i;
    RwUInt32            size;
    RwUInt32            version;

    if( !RwStreamFindChunk( stream, rwID_STRUCT, &size, &version ) )
        return nullptr;

    if( RwStreamRead( stream, &fl, sizeof( fl ) ) != sizeof( fl ) )
    {
        return nullptr;
    }
    logger::Log( "numFrames:" +
                 std::to_string( fl.numFrames ) );

    frameList->numFrames = fl.numFrames;

    frameList->frames = (RwFrame * *)malloc( sizeof( RwFrame* ) * fl.numFrames );
    for( i = 0; i < fl.numFrames; i++ )
    {
        rwStreamFrame      f;
        RwFrame* frame;
        RwMatrix* mat;

        if( RwStreamRead( stream, &f, sizeof( f ) ) != sizeof( f ) )
        {
            free( frameList->frames );
            return nullptr;
        }

        /* Create the frame */
        frame = _RwFrameCreate();

        if( !frame )
        {
            free( frameList->frames );
            return nullptr;
        }

        logger::Log( f.to_string() );

        mat = &frame->modelling;//RwFrameGetMatrix( frame );
        *RwMatrixGetRight( mat ) = f.right;
        *RwMatrixGetUp( mat ) = f.up;
        *RwMatrixGetAt( mat ) = f.at;
        *RwMatrixGetPos( mat ) = f.pos;

        /*if( rwMatrixIsOrthonormalPositive( mat, _ORTHONORMAL_TOL ) )
        {
            rwMatrixSetFlags( mat,
                              rwMatrixGetFlags( mat ) &
                              ~rwMATRIXINTERNALIDENTITY );
        }
        else
        {
            rwMatrixSetFlags( mat,
                              rwMatrixGetFlags( mat ) &
                              ~( rwMATRIXINTERNALIDENTITY |
                                 rwMATRIXTYPEORTHONORMAL ) );
        }*/

        frameList->frames[i] = frame;

        /* Set the frame pointer */
        if( f.parentIndex >= 0 )
        {
            _RwFrameAddChild( frameList->frames[f.parentIndex], frame );
        }
    }
}
