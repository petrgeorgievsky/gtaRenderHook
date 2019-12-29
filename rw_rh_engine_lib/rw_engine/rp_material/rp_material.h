#pragma once
#include <cstdint>
struct RpMaterial;
namespace rh::rw::engine {
RpMaterial *RpMaterialStreamRead( void *stream );

RpMaterial *RpMaterialCreate();

int32_t RpMaterialDestroy( RpMaterial *material );

} // namespace rw_rh_engine
