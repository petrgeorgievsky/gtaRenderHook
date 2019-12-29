#pragma once
#include <cstdint>
struct RwRaster;
namespace rh::rw::engine {
class RwRasterUnlockCmd
{
public:
    RwRasterUnlockCmd( RwRaster *raster );
    bool Execute();

private:
    RwRaster *m_pRaster = nullptr;
};
} // namespace rw_rh_engine
