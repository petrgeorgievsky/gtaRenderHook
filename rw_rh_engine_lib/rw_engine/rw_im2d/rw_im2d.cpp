#include "rw_im2d.h"
#include "../../RwRenderEngine.h"
#include <common_headers.h>

int32_t rh::rw::engine::RwIm2DRenderPrimitive( int32_t primType,
                                             RwIm2DVertex *vertices,
                                             int32_t numVertices )
{
    return g_pRwRenderEngine->Im2DRenderPrimitive( static_cast<RwPrimitiveType>( primType ),
                                                   vertices,
                                                   numVertices );
}
