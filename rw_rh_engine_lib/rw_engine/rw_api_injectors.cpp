#include "rw_api_injectors.h"
#include "rw_raster/rw_raster.h"
#include "rw_stream/rw_stream.h"
#include "rw_texture/rw_texture.h"
using namespace rh::rw::engine;

RwIOPointerTable rh::rw::engine::g_pIO_API = {RwStreamFindChunk, RwStreamRead};
uint32_t rh::rw::engine::g_oInternalRasterExtOffset = sizeof(RwRaster);
RwGlobalPointerTable rh::rw::engine::g_pGlobal_API
    = {nullptr,
       &g_oInternalRasterExtOffset,
       []( void *, RwResEntry **res, RwInt32 s, RwResEntryDestroyNotify ) {
           *res = static_cast<RwResEntry *>(
               malloc( sizeof( RwResEntry ) + static_cast<size_t>( s ) ) );
           return *res;
       }};
RwRasterPointerTable rh::rw::engine::g_pRaster_API
    = {reinterpret_cast<RwRasterCreate_FN>( rh::rw::engine::RwRasterCreate ),
       reinterpret_cast<RwRasterDestroy_FN>( rh::rw::engine::RwRasterDestroy )};
RwTexturePointerTable rh::rw::engine::g_pTexture_API
    = {rh::rw::engine::RwTextureCreate,
       []( RwTexture *texture, const RwChar *name ) {
           strncpy_s( texture->name, name, strlen( name ) );
           return texture;
       },
       []( RwTexture *texture, const RwChar *name ) {
           strncpy_s( texture->mask, name, strlen( name ) );
           return texture;
       }};
