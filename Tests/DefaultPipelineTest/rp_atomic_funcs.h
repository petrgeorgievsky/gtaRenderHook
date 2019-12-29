#pragma once
#include <RWUtils/RwAPI.h>

namespace RH_RWAPI
{

RpAtomic* _RpAtomicCreate( void );

RpAtomic* _RpAtomicSetFrame( RpAtomic* atomic, RwFrame* frame );

RpAtomic* _RpAtomicSetGeometry( RpAtomic* atomic, RpGeometry* geometry, RwUInt32 flags );

void _RpAtomicDestroy( RpAtomic* atomic );

}