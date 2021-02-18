//
// Created by peter on 19.11.2020.
//

#include "im3d_backend.h"
#include "im2d_backend.h"
#include "raster_backend.h"
#include <render_client/render_client.h>
namespace rh::rw::engine
{

int32_t Im3DRenderIndexedPrimitive( RwPrimitiveType primType, uint16_t *indices,
                                    int32_t numIndices )
{
    assert( gRenderClient );
    auto &im3d = gRenderClient->RenderState.Im3D;
    im3d.RenderIndexedPrimitive( primType, indices, numIndices );
    return 1;
}
void *Im3DTransform( void *pVerts, uint32_t numVerts, RwMatrix *ltm,
                     uint32_t flags )
{
    assert( gRenderClient );
    auto &im3d = gRenderClient->RenderState.Im3D;
    im3d.Transform( static_cast<RwIm3DVertex *>( pVerts ), numVerts, ltm,
                    flags );
    return pVerts;
}
int32_t Im3DEnd() { return 1; }
int32_t Im3DRenderPrimitive( RwPrimitiveType primType )
{
    assert( gRenderClient );
    auto &im3d = gRenderClient->RenderState.Im3D;
    im3d.RenderPrimitive( primType );
    return 1;
}
int32_t Im3DRenderLine( int32_t vert1, int32_t vert2 ) { return 1; }
int32_t Im3DRenderTriangle( int32_t vert1, int32_t vert2, int32_t vert3 )
{
    return 1;
}
} // namespace rh::rw::engine