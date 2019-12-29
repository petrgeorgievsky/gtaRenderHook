#pragma once
#include <cstdint>
struct RpMaterialList;
struct RpMaterial;
namespace rh::rw::engine {
void _rpMaterialListInitialize( RpMaterialList &matList );

void _rpMaterialListDeinitialize( RpMaterialList &matList );

bool _rpMaterialListSetSize( RpMaterialList &matList, int32_t size );

int32_t _rpMaterialListAppendMaterial( RpMaterialList &matList, RpMaterial *material );

bool _rpMaterialListStreamRead( void *stream, RpMaterialList &matList );
} // namespace rw_rh_engine
