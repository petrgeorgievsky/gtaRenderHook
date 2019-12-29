#pragma once
#include <RWUtils/RwAPI.h>

namespace RH_RWAPI
{

static void _rwFrameInternalInit( RwFrame* frame ) noexcept;

RwFrame* _RwFrameCreate( void ) noexcept;

void _rwSetHierarchyRoot( RwFrame* frame, RwFrame* root );

void _RwFrameUpdateObjects( RwFrame* frame ) noexcept;

void _RwFrameRemoveChild( RwFrame* child );

void _RwFrameAddChild( RwFrame* parent, RwFrame* child );

RwMatrix* _RwFrameGetLTM( RwFrame* frame );
void __rwFrameSyncHierarchyLTM( RwFrame* frame );
}