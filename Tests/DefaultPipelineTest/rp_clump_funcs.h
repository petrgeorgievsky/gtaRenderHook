#pragma once
#include <RWUtils/RwAPI.h>

namespace RH_RWAPI
{

RpClump* _RpClumpCreate( void ) noexcept;

RwBool _RpClumpDestroy( RpClump* clump );

RpClump* _RpClumpAddAtomic( RpClump* clump, RpAtomic* atomic ) noexcept;

}