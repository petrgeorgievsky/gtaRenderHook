#pragma once
#include <stdint.h>
struct RwRaster;
namespace rh::rw::engine {
RwRaster *RwRasterCreate(int32_t width, int32_t height, int32_t depth, int32_t flags);

int32_t RwRasterDestroy(RwRaster *raster);
} // namespace rw_rh_engine
