#pragma once
struct RwRaster;
#include <cstdint>
namespace rh::rw::engine
{

class RwRasterShowRasterCmd
{
public:
    RwRasterShowRasterCmd( RwRaster* raster, int32_t flags );
    bool Execute();
private:
    RwRaster* m_pRaster = nullptr;
    int32_t m_nFlags = 0;
};

}
