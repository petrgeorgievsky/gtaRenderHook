#pragma once
#include <RWUtils/RwAPI.h>

namespace RH_RWAPI 
{
static uint32_t rwMATERIALLISTGRANULARITY = 20;

void _rpMaterialListInitialize( RpMaterialList& matList );

void _rpMaterialListDeinitialize( RpMaterialList& matList );

bool _rpMaterialListSetSize( RpMaterialList& matList, RwInt32 size );

RwInt32 _rpMaterialListAppendMaterial( RpMaterialList& matList, RpMaterial* material );

}