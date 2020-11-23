#include "../rw_stream/rw_stream.h"
#include "rw_frame.h"
#include <DebugUtils/DebugLogger.h>
#include <common_headers.h>

struct rwStreamFrame
{
    RwV3d    right, up, at, pos;
    int32_t  parentIndex;
    uint32_t data;
};

rwFrameList *rh::rw::engine::_rwFrameListStreamRead( void *       stream,
                                                     rwFrameList *frameList )
{
    using logger = rh::debug::DebugLogger;

    logger::Log( "rwFrameListStreamRead start" );

    struct rwStreamFrameList
    {
        int32_t numFrames;
    } fl;
    int32_t  i;
    uint32_t size;
    uint32_t version;

    if ( !RwStreamFindChunk( stream, rwID_STRUCT, &size, &version ) )
        return nullptr;

    if ( RwStreamRead( stream, &fl, sizeof( fl ) ) != sizeof( fl ) )
        return nullptr;

    logger::Log( "numFrames:" + std::to_string( fl.numFrames ) );

    frameList->numFrames = fl.numFrames;

    frameList->frames = static_cast<RwFrame **>(
        malloc( sizeof( RwFrame * ) * static_cast<size_t>( fl.numFrames ) ) );

    for ( i = 0; i < fl.numFrames; i++ )
    {
        rwStreamFrame f;
        RwFrame *     frame;
        RwMatrix *    mat;

        if ( RwStreamRead( stream, &f, sizeof( f ) ) != sizeof( f ) )
        {
            free( frameList->frames );
            return nullptr;
        }

        /* Create the frame */
        frame = RwFrameCreate();

        if ( !frame )
        {
            free( frameList->frames );
            return nullptr;
        }

        mat        = &frame->modelling; // RwFrameGetMatrix( frame );
        mat->right = f.right;
        mat->up    = f.up;
        mat->at    = f.at;
        mat->pos   = f.pos;

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
        if ( f.parentIndex >= 0 )
        {
            rh::rw::engine::RwFrameAddChild( frameList->frames[f.parentIndex],
                                             frame );
        }
    }
    return frameList;
}
