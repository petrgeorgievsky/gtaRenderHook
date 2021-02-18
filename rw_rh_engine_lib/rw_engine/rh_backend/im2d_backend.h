#pragma once
#include <cstdint>
struct RwIm2DVertex;
namespace rh
{
namespace rw::engine
{

int32_t Im2DRenderPrimitiveFunction( int32_t primType, RwIm2DVertex *vertices,
                                     int32_t numVertices );
int32_t Im2DRenderIndexedPrimitiveFunction( int32_t       primType,
                                            RwIm2DVertex *vertices,
                                            int32_t       numVertices,
                                            int16_t *     indices,
                                            int32_t       numIndices );

} // namespace rw::engine
} // namespace rh