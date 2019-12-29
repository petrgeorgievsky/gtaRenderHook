#pragma once
#include "common_headers.h"

namespace rh::rw::engine
{
#define RwRasterGetFormatMacro(_raster) \
    ((((_raster)->cFormat) & (rwRASTERFORMATMASK >> 8)) << 8)
}
