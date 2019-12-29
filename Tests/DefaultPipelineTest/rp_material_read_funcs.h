#pragma once
#include <RWUtils/RwAPI.h>

namespace RH_RWAPI
{

RpMaterial* _RpMaterialStreamRead( void* stream );

}

RwTexture* _RwTextureRead( const RwChar* name, const RwChar* maskName );