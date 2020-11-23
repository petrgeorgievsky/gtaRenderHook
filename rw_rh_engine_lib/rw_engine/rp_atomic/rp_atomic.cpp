#include "rp_atomic.h"
#include "../rp_geometry/rp_geometry.h"
#include "../rw_frame/rw_frame.h"
#include "../rw_macro_constexpr.h"

#include <common_headers.h>
namespace rh::rw::engine
{
enum RpAtomicPrivateFlag
{
    rpATOMICPRIVATEWORLDBOUNDDIRTY = 0x01,
};
static RwObjectHasFrame *AtomicSync( RwObjectHasFrame *object )
{
    auto            atomic = reinterpret_cast<RpAtomic *>( object );
    RpInterpolator *interpolator;

    interpolator = &atomic->interpolator;
    /*if( interpolator->flags & rpINTERPOLATORDIRTYSPHERE )
{
_rpAtomicResyncInterpolatedSphere( atomic );
}*/

    /* This doesn't do much.
     * The work is done in the function that the world adds to this chain */
    rwObject::SetPrivateFlags( object, rwObject::GetPrivateFlags( object ) |
                                           rpATOMICPRIVATEWORLDBOUNDDIRTY );

    return ( object );
}
enum RpAtomicFlag
{
    rpATOMICCOLLISIONTEST = 0x01, /**<A generic collision flag to indicate
                                   * that the atomic should be considered
                                   * in collision tests.
                                   */
    rpATOMICRENDER = 0x04,        /**<The atomic is rendered if it is
                                   * in the view frustum.
                                   */

};
enum RpInterpolatorFlag
{
    rpINTERPOLATORDIRTYINSTANCE = 0x01,
    rpINTERPOLATORDIRTYSPHERE   = 0x02,
    rpINTERPOLATORNOFRAMEDIRTY  = 0x04
};
enum RpAtomicSetGeomFlag
{
    rpATOMICSAMEBOUNDINGSPHERE = 0x01, /**<The bounding-sphere for the
                                        * new geometry is assumed to be the
                                        * same as the old one, if any, and
                                        * should not be recalculated.
                                        */

};
RpAtomic *RpAtomicCreate()
{
    RpAtomic *atomic;

    atomic = static_cast<RpAtomic *>( malloc( sizeof( RpAtomic ) ) );
    if ( !atomic )
    {
        return nullptr;
    }
    constexpr auto rpATOMIC = 1;
    /* We don't care about the sub type */
    rwObject::HasFrameInitialize( atomic, rpATOMIC, 0, AtomicSync );

    /* NOTE: the repEntry is set to NULL because (a) if the geom is
     * non-animated then IT will hold the repEntry and (b) if the geom is
     * animated then the atomic will have a repEntry created during its
     * next pipeline execute. [A similar argument applies to the PS2
     * rwMeshCache plugin set up during rwPluginRegistryInitObject, below] */
    atomic->repEntry = nullptr;

    /* Flag the atomic as dirty -> ie vertices need expanding,
     * and the world space bounding sphere too...
     */
    rwObject::SetFlags( atomic, rpATOMICCOLLISIONTEST | rpATOMICRENDER );
    rwObject::SetPrivateFlags( atomic, rpATOMICPRIVATEWORLDBOUNDDIRTY );

    /* No frame yet */
    rh::rw::engine::RpAtomicSetFrame( atomic, nullptr );

    /* Set up clump status */
    atomic->geometry = nullptr;

    /* And the bounding sphere's (these get updated during frame
     * synchronization and instancing)
     */
    atomic->boundingSphere.radius   = 0.0F;
    atomic->boundingSphere.center.x = 0.0F;
    atomic->boundingSphere.center.y = 0.0F;
    atomic->boundingSphere.center.z = 0.0F;

    atomic->worldBoundingSphere.radius   = 0.0F;
    atomic->worldBoundingSphere.center.x = 0.0F;
    atomic->worldBoundingSphere.center.y = 0.0F;
    atomic->worldBoundingSphere.center.z = 0.0F;

    /* Set up the default render callback */
    // RpAtomicSetRenderCallBack( atomic, AtomicDefaultRenderCallBack );

    /* Set on the first frame of the geometry */
    atomic->interpolator.startMorphTarget = 0;
    atomic->interpolator.endMorphTarget   = 0;
    atomic->interpolator.time             = 1.0F;
    atomic->interpolator.recipTime        = 1.0F;
    atomic->interpolator.position         = 0.0F;
    atomic->interpolator.flags =
        ( rpINTERPOLATORDIRTYINSTANCE | rpINTERPOLATORDIRTYSPHERE );

    /* membership of clump */
    rwLLLink::Initialize( &atomic->inClumpLink );
    atomic->clump = nullptr;

    /* use the default atomic object pipeline */
    atomic->pipeline = nullptr;

    /* Not in any atomic sectors */
    rwLinkList::Initialize( &atomic->llWorldSectorsInAtomic );

    /* Initialize memory allocated to toolkits */
    // rwPluginRegistryInitObject( &atomicTKList, atomic );

    return ( atomic );
}

void RpAtomicDestroy( RpAtomic *atomic )
{
    if ( atomic->repEntry )
    {
        if ( atomic->repEntry->destroyNotify )
            atomic->repEntry->destroyNotify( atomic->repEntry );
        if ( atomic->repEntry->ownerRef )
        {
            *( atomic->repEntry->ownerRef ) = nullptr;
        }
        free( atomic->repEntry );
    }
    if ( atomic->geometry )
    {
        rh::rw::engine::RpGeometryDestroy( atomic->geometry );
    }
    free( atomic );
}

void __rwObjectHasFrameSetFrame( void *object, RwFrame *frame )
{
    auto ohf = reinterpret_cast<RwObjectHasFrame *>( object );

    if ( rwObject::GetParent( ohf ) )
    {
        rwLinkList::RemoveLLLink( &ohf->lFrame );
    }

    /* Set the pointer */
    rwObject::SetParent( object, frame );

    /* Add it to the frames list of objects */
    if ( frame )
    {
        rwLinkList::AddLLLink( &frame->objectList, &ohf->lFrame );

        /* Force the objects using this frame to be updated */
        rh::rw::engine::RwFrameUpdateObjects( frame );
    }
}

RpAtomic *RpAtomicSetFrame( RpAtomic *atomic, RwFrame *frame )
{
    __rwObjectHasFrameSetFrame( atomic, frame );

    /* World bounding sphere is no longer valid */
    rwObject::SetPrivateFlags( atomic, rwObject::GetPrivateFlags( atomic ) |
                                           rpATOMICPRIVATEWORLDBOUNDDIRTY );

    return ( atomic );
}

RpAtomic *RpAtomicSetGeometry( RpAtomic *atomic, RpGeometry *geometry,
                               uint32_t flags )
{
    if ( geometry != atomic->geometry )
    {
        // RwFrame* frame;

        if ( geometry )
        {
            /* Add ref the new geometry */
            RpGeometryAddRef( geometry );
        }

        if ( atomic->geometry )
        {
            /* Reduce refrence count on the old geometry */
            rh::rw::engine::RpGeometryDestroy( atomic->geometry );
        }

        /* The instanced copy will be updated when the
         * mesh serial numbers don't match during instancing
         */

        /* Point to the geometry used */
        atomic->geometry = geometry;

        if ( !( flags & rpATOMICSAMEBOUNDINGSPHERE ) )
        {
            if ( geometry )
            {
                atomic->boundingSphere =
                    geometry->morphTarget[0].boundingSphere;
            }

            // frame = (RwFrame *)rwObjectGetParent( atomic );
            // if( frame && RpAtomicGetWorld( atomic ) )
            //{
            //    /* Mark the frame as dirty so the ties get updated. */
            //    RwFrameUpdateObjects( frame );
            //}
        }

        /* The instanced copy will be updated when the
         * mesh serial numbers don't match during instancing
         */
    }

    return ( atomic );
}

} // namespace rh::rw::engine
