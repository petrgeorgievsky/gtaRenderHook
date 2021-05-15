#include "rw_frame.h"
#include "../rw_macro_constexpr.h"
#include <common_headers.h>
namespace rh::rw::engine
{
#define rwFRAMEPRIVATEHIERARCHYSYNCLTM 0x01
#define rwFRAMEPRIVATEHIERARCHYSYNCOBJ 0x02
#define rwFRAMEPRIVATESUBTREESYNCLTM 0x04
#define rwFRAMEPRIVATESUBTREESYNCOBJ 0x08
#define rwFRAMEPRIVATESTATIC 0x10
static void rwFrameInternalInit( RwFrame *frame )
{
    constexpr auto rwFRAME = 0;
    rwObject::Initialize( frame, rwFRAME, 0 );
    rwLinkList::Initialize( &frame->objectList ); /* No objects attached here */

    /* Set up the structure */
    frame->modelling = { { 1, 0, 0 }, 0, { 0, 1, 0 }, 0,
                         { 0, 0, 1 }, 0, { 0, 0, 0 }, 0x3F800000 };
    frame->ltm       = { { 1, 0, 0 }, 0, { 0, 1, 0 }, 0,
                   { 0, 0, 1 }, 0, { 0, 0, 0 }, 0x3F800000 };
    // rwMatrixInitializeIdentity( &frame->modelling, rwMATRIXTYPEORTHONORMAL );
    // rwMatrixInitializeIdentity( &frame->ltm, rwMATRIXTYPEORTHONORMAL );

    /* Set up the position in the hierarchy */
    frame->child = nullptr;
    frame->next  = nullptr;

    /* Point to root */
    frame->root = frame;
}

RwFrame *RwFrameCreate()
{
    RwFrame *frame;

    frame = hAlloc<RwFrame>( "Frame" );
    if ( !frame )
    {
        return nullptr;
    }

    rwFrameInternalInit( frame );

    /* All done */
    return ( frame );
}

void _rwSetHierarchyRoot( RwFrame *frame, RwFrame *root )
{
    frame->root = root;

    frame = frame->child;
    while ( frame )
    {
        _rwSetHierarchyRoot( frame, root );
        frame = frame->next;
    }
}

void RwFrameAddChild( RwFrame *parent, RwFrame *child )
{
    if ( rwFrame::GetParent( child ) )
    {
        rh::rw::engine::RwFrameRemoveChild( child );
    }

    /* add as child and re-parent */
    child->next   = parent->child;
    parent->child = child;
    rwObject::SetParent( child, parent );
    _rwSetHierarchyRoot( child, parent->root );

    /* Handle if its dirty */
    if ( rwObject::TestPrivateFlags( child,
                                     rwFRAMEPRIVATEHIERARCHYSYNCLTM |
                                         rwFRAMEPRIVATEHIERARCHYSYNCOBJ ) )
    {
        rwLinkList::RemoveLLLink( &child->inDirtyListLink );
        /* clear flag */
        rwObject::SetPrivateFlags(
            child,
            static_cast<uint8_t>( rwObject::GetPrivateFlags( child ) &
                                  ~( rwFRAMEPRIVATEHIERARCHYSYNCLTM |
                                     rwFRAMEPRIVATEHIERARCHYSYNCOBJ ) ) );
    }

    /* The child's local transformation matrix is definitely dirty */
    rh::rw::engine::RwFrameUpdateObjects( child );
}

void RwFrameRemoveChild( RwFrame *child )
{
    RwFrame *curFrame = nullptr;

    assert( child != nullptr );

    /* Need to do something */
    curFrame = ( rwFrame::GetParent( child ) )->child;
    assert( curFrame != nullptr );

    /* Check if its the first child */
    if ( curFrame == child )
    {
        rwFrame::GetParent( child )->child = child->next;
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
    rwObject::SetParent( child, nullptr );
    child->next = nullptr;

    /* Set the hierarchy root */
    _rwSetHierarchyRoot( child, child );

    /* Make it dirty */
    rh::rw::engine::RwFrameUpdateObjects( child );
}

void RwFrameUpdateObjects( RwFrame *frame ) noexcept
{
    uint8_t oldFlags;

    oldFlags = rwObject::GetPrivateFlags( frame->root );
    // if( !( oldFlags & ( rwFRAMEPRIVATEHIERARCHYSYNCLTM |
    //                    rwFRAMEPRIVATEHIERARCHYSYNCOBJ ) ) )
    //{
    //    /* Not in the dirty list - add it */
    //    rwLinkListAddLLLink( &RWSRCGLOBAL( dirtyFrameList ),
    //                         &frame->root->inDirtyListLink );
    //}

    /* whole hierarchy needs resync */
    rwObject::SetPrivateFlags(
        frame->root,
        static_cast<uint8_t>( oldFlags | ( rwFRAMEPRIVATEHIERARCHYSYNCLTM |
                                           rwFRAMEPRIVATEHIERARCHYSYNCOBJ ) ) );

    /* this frame in particular */
    rwObject::SetPrivateFlags(
        frame, static_cast<uint8_t>( rwObject::GetPrivateFlags( frame ) |
                                     ( rwFRAMEPRIVATESUBTREESYNCLTM |
                                       rwFRAMEPRIVATESUBTREESYNCOBJ ) ) );
}

RwMatrix *RwFrameGetLTM( RwFrame *frame )
{
    /* If something has changed, sync the hierarchy */
    if ( rwObject::TestPrivateFlags( frame->root,
                                     rwFRAMEPRIVATEHIERARCHYSYNCLTM ) )
    {
        /* Sync the whole hierarchy */
        rh::rw::engine::_rwFrameSyncHierarchyLTM( frame->root );
    }

    return ( &frame->ltm );
}

static void FrameSyncHierarchyLTMRecurse( RwFrame *frame, int32_t flags )
{
    /* NULL is a valid; termination condition */
    while ( frame )
    {
        int32_t accumflags = flags | rwObject::GetPrivateFlags( frame );

        if ( accumflags & rwFRAMEPRIVATESUBTREESYNCLTM )
        {
            /* Work out the new local transformation matrix */
            RwMatrix *parent_ltm    = &( ( rwFrame::GetParent( frame ) )->ltm );
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
            frame->ltm.right = { DirectX::XMVectorGetX( res_transform.r[0] ),
                                 DirectX::XMVectorGetY( res_transform.r[0] ),
                                 DirectX::XMVectorGetZ( res_transform.r[0] ) };
            frame->ltm.up    = { DirectX::XMVectorGetX( res_transform.r[1] ),
                              DirectX::XMVectorGetY( res_transform.r[1] ),
                              DirectX::XMVectorGetZ( res_transform.r[1] ) };
            frame->ltm.at    = { DirectX::XMVectorGetX( res_transform.r[2] ),
                              DirectX::XMVectorGetY( res_transform.r[2] ),
                              DirectX::XMVectorGetZ( res_transform.r[2] ) };
            frame->ltm.pos   = { DirectX::XMVectorGetX( res_transform.r[3] ),
                               DirectX::XMVectorGetY( res_transform.r[3] ),
                               DirectX::XMVectorGetZ( res_transform.r[3] ) };
            /*RwMatrixMultiply( &frame->ltm,
&frame->modelling,
&( ( RwFrameGetParent( frame ) )->ltm ) );*/
            /* clear flag */
            rwObject::SetPrivateFlags(
                frame,
                static_cast<uint8_t>( rwObject::GetPrivateFlags( frame ) &
                                      ~( rwFRAMEPRIVATESUBTREESYNCLTM ) ) );
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

void _rwFrameSyncHierarchyLTM( RwFrame *frame )
{
    uint8_t oldFlags;

    oldFlags = rwObject::GetPrivateFlags( frame );

    if ( oldFlags & rwFRAMEPRIVATESUBTREESYNCLTM )
    {
        /* Root of hierarchy has no parent matrix - different from rest of
         * hierarchy
         */
        // RwMatrixCopy( &frame->ltm, &frame->modelling );
        frame->ltm.right = frame->modelling.right;
        frame->ltm.up    = frame->modelling.up;
        frame->ltm.at    = frame->modelling.at;
        frame->ltm.pos   = frame->modelling.pos;
    }

    /* Do the children */
    FrameSyncHierarchyLTMRecurse( frame->child, oldFlags );

    /* clear flag */
    rwObject::SetPrivateFlags(
        frame,
        static_cast<uint8_t>( oldFlags & ~( rwFRAMEPRIVATEHIERARCHYSYNCLTM |
                                            rwFRAMEPRIVATESUBTREESYNCLTM ) ) );
}

} // namespace rh::rw::engine
