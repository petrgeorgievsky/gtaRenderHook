#pragma once
struct RwRaster;
#include <stdint.h>
namespace rh::rw::engine {

class RwRasterCreateCmd
{
public:
    RwRasterCreateCmd(RwRaster *raster, uint32_t flags);
    bool Execute();

private:
    RwRaster *m_pRaster;
    uint32_t m_nFlags;
};

} // namespace rw_rh_engine
