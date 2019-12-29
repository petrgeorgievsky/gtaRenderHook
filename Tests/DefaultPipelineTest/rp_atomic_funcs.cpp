#include "rp_atomic_funcs.h"
#include "rp_geometry_funcs.h"
#include "rw_frame_funcs.h"

static RwObjectHasFrame* _AtomicSync( RwObjectHasFrame* object )
{
    RpAtomic* atomic = (RpAtomic*)object;
    RpInterpolator* interpolator;

    interpolator = &atomic->interpolator;
    /*if( interpolator->flags & rpINTERPOLATORDIRTYSPHERE )
    {
        _rpAtomicResyncInterpolatedSphere( atomic );
    }*/

    /* This doesn't do much.
     * The work is done in the function that the world adds to this chain */
    rwObjectSetPrivateFlags( object,
                             rwObjectGetPrivateFlags( object ) |
                             rpATOMICPRIVATEWORLDBOUNDDIRTY );

    return ( object );
}
void __rwObjectHasFrameSetFrame( void* object, RwFrame* frame )
{
    RwObjectHasFrame* ohf = (RwObjectHasFrame*)object;

    if( rwObjectGetParent( ohf ) )
    {
        rwLinkListRemoveLLLink( &ohf->lFrame );
    }

    /* Set the pointer */
    rwObjectSetParent( object, frame );

    /* Add it to the frames list of objects */
    if( frame )
    {
        rwLinkListAddLLLink( &frame->objectList, &ohf->lFrame );

        /* Force the objects using this frame to be updated */
        RH_RWAPI::_RwFrameUpdateObjects( frame );
    }
}



RpAtomic* RH_RWAPI::_RpAtomicCreate( void )

{
    RpAtomic* atomic;

    atomic = (RpAtomic*)malloc( sizeof( RpAtomic ) );
    if( !atomic )
    {
        return nullptr;
    }

    /* We don't care about the sub type */
    rwObjectHasFrameInitialize( atomic, rpATOMIC, 0, _AtomicSync );

    /* NOTE: the repEntry is set to NULL because (a) if the geom is
     * non-animated then IT will hold the repEntry and (b) if the geom is
     * animated then the atomic will have a repEntry created during its
     * next pipeline execute. [A similar argument applies to the PS2
     * rwMeshCache plugin set up during rwPluginRegistryInitObject, below] */
    atomic->repEntry = (RwResEntry*)NULL;

    /* Flag the atomic as dirty -> ie vertices need expanding,
     * and the world space bounding sphere too...
     */
    rwObjectSetFlags( atomic, rpATOMICCOLLISIONTEST | rpATOMICRENDER );
    rwObjectSetPrivateFlags( atomic, rpATOMICPRIVATEWORLDBOUNDDIRTY );

    /* No frame yet */
    _RpAtomicSetFrame( atomic, nullptr );

    /* Set up clump status */
    atomic->geometry = (RpGeometry*)NULL;

    /* And the bounding sphere's (these get updated during frame
     * synchronization and instancing)
     */
    atomic->boundingSphere.radius = (RwReal)( 0 );
    atomic->boundingSphere.center.x = (RwReal)( 0 );
    atomic->boundingSphere.center.y = (RwReal)( 0 );
    atomic->boundingSphere.center.z = (RwReal)( 0 );

    atomic->worldBoundingSphere.radius = (RwReal)( 0 );
    atomic->worldBoundingSphere.center.x = (RwReal)( 0 );
    atomic->worldBoundingSphere.center.y = (RwReal)( 0 );
    atomic->worldBoundingSphere.center.z = (RwReal)( 0 );

    /* Set up the default render callback */
    //RpAtomicSetRenderCallBack( atomic, AtomicDefaultRenderCallBack );

    /* Set on the first frame of the geometry */
    atomic->interpolator.startMorphTarget = 0;
    atomic->interpolator.endMorphTarget = 0;
    atomic->interpolator.time = (RwReal)( 1.0 );
    atomic->interpolator.recipTime = (RwReal)( 1.0 );
    atomic->interpolator.position = (RwReal)( 0.0 );
    atomic->interpolator.flags = (RwInt32)
        ( rpINTERPOLATORDIRTYINSTANCE | rpINTERPOLATORDIRTYSPHERE );

    /* membership of clump */
    rwLLLinkInitialize( &atomic->inClumpLink );
    atomic->clump = (RpClump*)NULL;

    /* use the default atomic object pipeline */
    atomic->pipeline = (RxPipeline*)NULL;

    /* Not in any atomic sectors */
    rwLinkListInitialize( &atomic->llWorldSectorsInAtomic );

    /* Initialize memory allocated to toolkits */
    //rwPluginRegistryInitObject( &atomicTKList, atomic );

    return ( atomic );
}

RpAtomic* RH_RWAPI::_RpAtomicSetFrame( RpAtomic* atomic, RwFrame* frame )
{
    __rwObjectHasFrameSetFrame( atomic, frame );

    /* World bounding sphere is no longer valid */
    rwObjectSetPrivateFlags( atomic, rwObjectGetPrivateFlags( atomic ) |
                             rpATOMICPRIVATEWORLDBOUNDDIRTY );

    return ( atomic );
}

RpAtomic* RH_RWAPI::_RpAtomicSetGeometry( RpAtomic* atomic, RpGeometry* geometry, RwUInt32 flags )
{
    if( geometry != atomic->geometry )
    {
        //RwFrame* frame;

        if( geometry )
        {
            /* Add ref the new geometry */
            _RpGeometryAddRef( geometry );
        }

        if( atomic->geometry )
        {
            /* Reduce refrence count on the old geometry */
            _RpGeometryDestroy( atomic->geometry );
        }

        /* The instanced copy will be updated when the
         * mesh serial numbers don't match during instancing
         */

         /* Point to the geometry used */
        atomic->geometry = geometry;

        if( !( flags & rpATOMICSAMEBOUNDINGSPHERE ) )
        {
            if( geometry )
            {
                atomic->boundingSphere =
                    geometry->morphTarget[0].boundingSphere;
            }

            //frame = (RwFrame *)rwObjectGetParent( atomic );
            //if( frame && RpAtomicGetWorld( atomic ) )
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

void RH_RWAPI::_RpAtomicDestroy( RpAtomic* atomic )
{
    if( atomic->repEntry )
    {
        if( atomic->repEntry->destroyNotify )
            atomic->repEntry->destroyNotify( atomic->repEntry );
        if( atomic->repEntry->ownerRef )
        {
            *( atomic->repEntry->ownerRef ) = (RwResEntry*)NULL;
        }
        free( atomic->repEntry );
    }
    if( atomic->geometry )
    {
        RH_RWAPI::_RpGeometryDestroy( atomic->geometry );
    }
    free( atomic );
}
