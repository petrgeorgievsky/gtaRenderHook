#pragma once
#include <stdint.h>
struct RwD3D9Vertex;
typedef RwD3D9Vertex RwIm2DVertex;
namespace rh::rw::engine {

int32_t RwIm2DRenderPrimitive( int32_t primType, RwIm2DVertex *vertices, int32_t numVertices );
}
