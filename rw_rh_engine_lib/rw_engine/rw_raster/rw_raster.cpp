#include "rw_raster.h"
#include "../rh_backend/raster_backend.h"
#include "../system_funcs/rw_device_system_globals.h"
#include <rw_engine/system_funcs/rw_device_standards.h>

#include <common_headers.h>
namespace rh::rw::engine
{

RwRaster *RwRasterCreate( int32_t width, int32_t height, int32_t depth,
                          int32_t flags )
{
    auto *raster = static_cast<RwRaster *>(
        malloc( sizeof( RwRaster ) + sizeof( BackendRasterExt ) ) );
    new ( raster ) RwRaster{};

    if ( raster == nullptr )
        return nullptr;

    raster->privateFlags = 0;
    raster->cFlags       = 0;
    raster->width        = width;
    raster->height       = height;
    raster->nOffsetX     = 0;
    raster->nOffsetY     = 0;
    raster->depth        = depth;
    raster->parent       = raster; /* It contains its own pixels */
    raster->cpPixels     = nullptr;
    raster->palette      = nullptr;
    const RwStandardFunc RasterCreateFunc =
        gRwDeviceGlobals.Standards[rwSTANDARDRASTERCREATE];

    if ( !RasterCreateFunc( nullptr, raster, flags ) )
    {
        free( raster );

        return nullptr;
    }

    return raster;
}

int32_t RwRasterDestroy( RwRaster *raster )
{
    if ( raster == nullptr )
        return false;

    const RwStandardFunc RasterDestroyFunc =
        gRwDeviceGlobals.Standards[rwSTANDARDRASTERDESTROY];
    RasterDestroyFunc( nullptr, raster, 0 );

    free( raster );
    return true;
}

} // namespace rh::rw::engine
