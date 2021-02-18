#include "rw_api_injectors.h"
#include "common_headers.h"
#include "rw_raster/rw_raster.h"
#include "rw_stream/rw_stream.h"
#include "rw_texture/rw_texture.h"

namespace rh::rw::engine
{

RwTexture *RwTextureSetNameStub( RwTexture *texture, const char *name )
{
    strncpy_s( texture->name, name, strlen( name ) );
    return texture;
}

RwTexture *RwTextureSetMaskStub( RwTexture *texture, const char *name )
{
    strncpy_s( texture->mask, name, strlen( name ) );
    return texture;
}

RwTexturePointerTable g_pTexture_API = { RwTextureCreate, RwTextureSetNameStub,
                                         RwTextureSetMaskStub };
RwIOPointerTable      g_pIO_API      = { RwStreamFindChunk, RwStreamRead };

RwRasterPointerTable g_pRaster_API = {
    reinterpret_cast<RwRasterCreate_FN>( RwRasterCreate ),
    reinterpret_cast<RwRasterDestroy_FN>( RwRasterDestroy ) };
} // namespace rh::rw::engine