#include "rw_frame_funcs.h"

#define rwMatrixSetFlags( m, flagsbit ) ( ( m )->flags = ( flagsbit ) )
#define rwMatrixGetFlags( m ) ( ( m )->flags )

#define RwMatrixSetIdentityMacro( m )                                          \
    MACRO_START                                                                \
    {                                                                          \
        ( m )->right.x = ( m )->up.y = ( m )->at.z = ( RwReal )( ( 1.0 ) );    \
        ( m )->right.y = ( m )->right.z = ( m )->up.x = ( RwReal )( ( 0.0 ) ); \
        ( m )->up.z = ( m )->at.x = ( m )->at.y = ( RwReal )( ( 0.0 ) );       \
        ( m )->pos.x = ( m )->pos.y = ( m )->pos.z = ( RwReal )( ( 0.0 ) );    \
        rwMatrixSetFlags( ( m ), rwMatrixGetFlags( m ) |                       \
                                     ( rwMATRIXINTERNALIDENTITY |              \
                                       rwMATRIXTYPEORTHONORMAL ) );            \
    }                                                                          \
    MACRO_STOP

#define RwMatrixSetIdentity( m ) RwMatrixSetIdentityMacro( m )

#define rwMatrixInitialize( m, t )                                             \
    MACRO_START { rwMatrixSetFlags( ( m ), ( t ) ); }                          \
    MACRO_STOP

#define rwMatrixInitializeIdentity( m, t )                                     \
    MACRO_START                                                                \
    {                                                                          \
        rwMatrixInitialize( ( m ), ( t ) );                                    \
        RwMatrixSetIdentity( ( m ) );                                          \
    }                                                                          \
    MACRO_STOP

void RH_RWAPI::_rwFrameInternalInit( RwFrame *frame ) noexcept
{
    rwObjectInitialize( frame, rwFRAME, 0 );
    rwLinkListInitialize( &frame->objectList ); /* No objects attached here */

    /* Set up the structure */
    rwMatrixInitializeIdentity( &frame->modelling, rwMATRIXTYPEORTHONORMAL );
    rwMatrixInitializeIdentity( &frame->ltm, rwMATRIXTYPEORTHONORMAL );

    /* Set up the position in the hierarchy */
    frame->child = nullptr;
    frame->next  = nullptr;

    /* Point to root */
    frame->root = frame;
}

RwFrame *RH_RWAPI::_RwFrameCreate( void ) noexcept
{
    RwFrame *frame = (RwFrame *)malloc( sizeof( RwFrame ) );
    _rwFrameInternalInit( frame );
    return frame;
}

void RH_RWAPI::_rwSetHierarchyRoot( RwFrame *frame, RwFrame *root )
{
    frame->root = root;

    frame = frame->child;
    while ( frame )
    {
        _rwSetHierarchyRoot( frame, root );
        frame = frame->next;
    }
}

void RH_RWAPI::_RwFrameUpdateObjects( RwFrame *frame ) noexcept

{
    RwUInt32 oldFlags;

    oldFlags = rwObjectGetPrivateFlags( frame->root );
    // if( !( oldFlags & ( rwFRAMEPRIVATEHIERARCHYSYNCLTM |
    //                    rwFRAMEPRIVATEHIERARCHYSYNCOBJ ) ) )
    //{
    //    /* Not in the dirty list - add it */
    //    rwLinkListAddLLLink( &RWSRCGLOBAL( dirtyFrameList ),
    //                         &frame->root->inDirtyListLink );
    //}

    /* whole hierarchy needs resync */
    rwObjectSetPrivateFlags( frame->root,
                             oldFlags | ( rwFRAMEPRIVATEHIERARCHYSYNCLTM |
                                          rwFRAMEPRIVATEHIERARCHYSYNCOBJ ) );

    /* this frame in particular */
    rwObjectSetPrivateFlags( frame, rwObjectGetPrivateFlags( frame ) |
                                        ( rwFRAMEPRIVATESUBTREESYNCLTM |
                                          rwFRAMEPRIVATESUBTREESYNCOBJ ) );
}

void RH_RWAPI::_RwFrameRemoveChild( RwFrame *child )
{
    RwFrame *curFrame = nullptr;

    assert( child != nullptr );

    /* Need to do something */
    curFrame = ( RwFrameGetParent( child ) )->child;
    assert( curFrame != nullptr );

    /* Check if its the first child */
    if ( curFrame == child )
    {
        RwFrameGetParent( child )->child = child->next;
    }
    else
    {
        while ( curFrame->next != child )
        {
            curFrame = curFrame->next;
        }

        /* Remove it from the sibling list */
        curFrame->next = child->next;
    }

    /* Now its a root it also has no siblings */
    rwObjectSetParent( child, nullptr );
    child->next = nullptr;

    /* Set the hierarchy root */
    _rwSetHierarchyRoot( child, child );

    /* Make it dirty */
    _RwFrameUpdateObjects( child );
}

void RH_RWAPI::_RwFrameAddChild( RwFrame *parent, RwFrame *child )
{
    if ( RwFrameGetParent( child ) )
    {
        _RwFrameRemoveChild( child );
    }

    /* add as child and re-parent */
    child->next   = parent->child;
    parent->child = child;
    rwObjectSetParent( child, parent );
    _rwSetHierarchyRoot( child, parent->root );

    /* Handle if its dirty */
    if ( rwObjectTestPrivateFlags( child, rwFRAMEPRIVATEHIERARCHYSYNCLTM |
                                              rwFRAMEPRIVATEHIERARCHYSYNCOBJ ) )
    {
        rwLinkListRemoveLLLink( &child->inDirtyListLink );
        /* clear flag */
        rwObjectSetPrivateFlags( child,
                                 rwObjectGetPrivateFlags( child ) &
                                     ~( rwFRAMEPRIVATEHIERARCHYSYNCLTM |
                                        rwFRAMEPRIVATEHIERARCHYSYNCOBJ ) );
    }

    /* The child's local transformation matrix is definitely dirty */
    _RwFrameUpdateObjects( child );
}

RwMatrix *RH_RWAPI::_RwFrameGetLTM( RwFrame *frame )
{
    /* If something has changed, sync the hierarchy */
    if ( rwObjectTestPrivateFlags( frame->root,
                                   rwFRAMEPRIVATEHIERARCHYSYNCLTM ) )
    {
        /* Sync the whole hierarchy */
        __rwFrameSyncHierarchyLTM( frame->root );
    }

    return ( &frame->ltm );
}

static void FrameSyncHierarchyLTMRecurse( RwFrame *frame, RwInt32 flags )
{

    /* NULL is a valid; termination condition */
    while ( frame )
    {
        RwInt32 accumflags = flags | rwObjectGetPrivateFlags( frame );

        if ( accumflags & rwFRAMEPRIVATESUBTREESYNCLTM )
        {
            /* Work out the new local transformation matrix */
            RwMatrix *parent_ltm    = &( ( RwFrameGetParent( frame ) )->ltm );
            RwMatrix *local_ltm     = &frame->ltm;
            DirectX::XMMATRIX local = {
                local_ltm->right.x, local_ltm->right.y, local_ltm->right.z, 0,
                local_ltm->up.x,    local_ltm->up.y,    local_ltm->up.z,    0,
                local_ltm->at.x,    local_ltm->at.y,    local_ltm->at.z,    0,
                local_ltm->pos.x,   local_ltm->pos.y,   local_ltm->pos.z,   1 };
            DirectX::XMMATRIX parent = {
                parent_ltm->right.x, parent_ltm->right.y,
                parent_ltm->right.z, 0,
                parent_ltm->up.x,    parent_ltm->up.y,
                parent_ltm->up.z,    0,
                parent_ltm->at.x,    parent_ltm->at.y,
                parent_ltm->at.z,    0,
                parent_ltm->pos.x,   parent_ltm->pos.y,
                parent_ltm->pos.z,   1 };
            DirectX::XMMATRIX res_transform = local * parent;
            frame->ltm.right = { res_transform.r[0].vector4_f32[0],
                                 res_transform.r[0].vector4_f32[1],
                                 res_transform.r[0].vector4_f32[2] };
            frame->ltm.up    = { res_transform.r[1].vector4_f32[0],
                              res_transform.r[1].vector4_f32[1],
                              res_transform.r[1].vector4_f32[2] };
            frame->ltm.at    = { res_transform.r[2].vector4_f32[0],
                              res_transform.r[2].vector4_f32[1],
                              res_transform.r[2].vector4_f32[2] };
            frame->ltm.pos   = { res_transform.r[2].vector4_f32[0],
                               res_transform.r[2].vector4_f32[1],
                               res_transform.r[2].vector4_f32[2] };
            /*RwMatrixMultiply( &frame->ltm,
                              &frame->modelling,
                              &( ( RwFrameGetParent( frame ) )->ltm ) );*/
            /* clear flag */
            rwObjectSetPrivateFlags( frame,
                                     rwObjectGetPrivateFlags( frame ) &
                                         ~( rwFRAMEPRIVATESUBTREESYNCLTM ) );
        }

        /* Depth first */
        /* Child has dirty status including this frame,
         * sibling has dirty status of parent (parameter in)
         */
        FrameSyncHierarchyLTMRecurse( frame->child, accumflags );

        /* tail recursion */
        frame = frame->next;
    }
}

void RH_RWAPI::__rwFrameSyncHierarchyLTM( RwFrame *frame )

{
    RwInt32 oldFlags;

    oldFlags = rwObjectGetPrivateFlags( frame );

    if ( oldFlags & rwFRAMEPRIVATESUBTREESYNCLTM )
    {
        /* Root of hierarchy has no parent matrix - different from rest of
         * hierarchy */
        // RwMatrixCopy( &frame->ltm, &frame->modelling );
        frame->ltm.right = frame->modelling.right;
        frame->ltm.up    = frame->modelling.up;
        frame->ltm.at    = frame->modelling.at;
        frame->ltm.pos   = frame->modelling.pos;
    }

    /* Do the children */
    FrameSyncHierarchyLTMRecurse( frame->child, oldFlags );

    /* clear flag */
    rwObjectSetPrivateFlags( frame,
                             oldFlags & ~( rwFRAMEPRIVATEHIERARCHYSYNCLTM |
                                           rwFRAMEPRIVATESUBTREESYNCLTM ) );
}
