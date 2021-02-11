#include "raster_backend.h"
#include "../system_funcs/rw_device_system_globals.h"
#include "Engine/Common/IImageBuffer.h"
#include "Engine/Common/IImageView.h"
#include "common_headers.h"
#include "material_backend.h"

#include <cassert>
#include <rw_engine/system_funcs/raster_unload_cmd.h>

namespace rh::rw::engine
{

int32_t gBackendRasterExtOffset = 0;

void *BackendRasterCtor( void *object, [[maybe_unused]] int32_t offsetInObject,
                         [[maybe_unused]] int32_t sizeInObject )
{
    auto *rasExt     = GetBackendRasterExt( static_cast<RwRaster *>( object ) );
    rasExt->mImageId = 0xBADF00D;
    return ( object );
}

void *BackendRasterDtor( void *object, [[maybe_unused]] int32_t offsetInObject,
                         [[maybe_unused]] int32_t sizeInObject )
{
    auto *rasExt = GetBackendRasterExt( static_cast<RwRaster *>( object ) );
    auto  img_id = rasExt->mImageId;
    if ( rasExt->mImageId == 0xBADF00D )
        return ( object );

    assert( gRenderClient );
    auto &               client = *gRenderClient;
    RasterDestroyCmdImpl cmd( client.GetTaskQueue() );
    cmd.Invoke( img_id );

    rasExt->mImageId = 0xBADF00D;

    /* Phew! */
    return ( object );
}

int32_t BackendRasterPluginAttach()
{
    gBackendRasterExtOffset = gRwDeviceGlobals.PluginFuncs.RasterRegisterPlugin(
        sizeof( BackendRasterExt ), rwID_RASTER_BACKEND, BackendRasterCtor,
        BackendRasterDtor, nullptr );
    return gBackendRasterExtOffset > 0;
}

BackendRasterExt *GetBackendRasterExt( RwRaster *raster )
{
    assert( raster );
    auto *internalRaster = reinterpret_cast<BackendRasterExt *>(
        ( reinterpret_cast<uint8_t *>( raster ) ) + gBackendRasterExtOffset );

    return internalRaster;
}

} // namespace rh::rw::engine
