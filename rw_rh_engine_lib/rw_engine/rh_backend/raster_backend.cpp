#include "raster_backend.h"
#include "common_headers.h"

#include <cassert>
#include <render_client/render_client.h>
#include <rw_engine/system_funcs/raster_unload_cmd.h>
#include <rw_engine/system_funcs/rw_device_system_globals.h> // PluginPtrTable

namespace rh::rw::engine
{
int32_t BackendRasterPlugin::Offset = -1;

BackendRasterPlugin::BackendRasterPlugin( const PluginPtrTable &plugin_cb )
{
    auto ctor = []( void *object, int32_t offsetInObject,
                    int32_t sizeInObject ) {
        auto ext = GetAddress( static_cast<RwRaster *>( object ) );
        new ( ext ) BackendRasterExt;
        return object;
    };
    auto dtor = []( void *object, int32_t offsetInObject,
                    int32_t sizeInObject ) {
        auto &ext = GetData( static_cast<RwRaster *>( object ) );
        ext.~BackendRasterExt();
        return object;
    };
    Offset = plugin_cb.RasterRegisterPlugin(
        sizeof( BackendRasterExt ), rwID_RASTER_BACKEND, ctor, dtor, nullptr );
    assert( Offset > 0 );
}

uint8_t *BackendRasterPlugin::GetAddress( RwRaster *raster )
{
    assert( raster && Offset > 0 );
    return ( reinterpret_cast<uint8_t *>( raster ) ) + Offset;
}

BackendRasterExt &BackendRasterPlugin::GetData( RwRaster *raster )
{
    return *reinterpret_cast<BackendRasterExt *>( GetAddress( raster ) );
}

BackendRasterExt::~BackendRasterExt()
{
    if ( mImageId == BackendRasterPlugin::NullRasterId )
        return;

    // Destroy raster
    // TODO: Better way could be to free rasters in batches, per-frame, need
    // to test this
    assert( gRenderClient );
    if ( gRenderClient )
    {
        auto &               client = *gRenderClient;
        RasterDestroyCmdImpl cmd( client.GetTaskQueue() );
        cmd.Invoke( mImageId );
    }

    mImageId = BackendRasterPlugin::NullRasterId;
}
BackendRasterExt::BackendRasterExt()
{
    mImageId = BackendRasterPlugin::NullRasterId;
}
} // namespace rh::rw::engine
