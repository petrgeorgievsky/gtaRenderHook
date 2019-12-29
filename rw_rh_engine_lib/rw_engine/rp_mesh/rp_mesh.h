#pragma once
#include <cstdint>
struct RpMeshHeader;
struct RpMaterialList;
struct RpGeometry;
namespace rh::rw::engine {
RpMeshHeader *rpMeshRead( void *stream,
                          const RpGeometry *geometry,
                          const RpMaterialList *matList ) noexcept;

bool readGeometryMesh( void *stream, RpGeometry *geometry ) noexcept;

extern uint32_t g_nMeshSerialNum;
} // namespace rw_rh_engine
