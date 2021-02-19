#pragma once
#include "i_rp_geometry.h"
#include <cstdint>
#include <functional>
#include <span>
#include <vector>

namespace rh::engine
{
class IBuffer;
} // namespace rh::engine
struct RpAtomic;
struct RpMeshHeader;
struct RwResEntry;
struct RpMorphTarget;
struct RwTexCoords;
struct RwRGBA;
struct RpTriangle;

namespace rh::rw::engine
{

struct ResEnty : RwResEntry
{
    uint64_t meshData;
    uint16_t batchId;
    uint16_t frameId;
};

enum RenderStatus
{
    Failure,
    NotInstanced,
    Instanced
};

RenderStatus InstanceAtomic( RpAtomic *atomic, RpGeometryInterface *geom_io );

void MeshGetNumVerticesMinIndex( const uint16_t *indices, uint32_t size,
                                 uint32_t &numVertices, uint32_t &min );

void DrawAtomic( RpAtomic *atomic, RpGeometryInterface *geom_io,
                 const std::function<void( ResEnty *entry )> &render_callback );

} // namespace rh::rw::engine
