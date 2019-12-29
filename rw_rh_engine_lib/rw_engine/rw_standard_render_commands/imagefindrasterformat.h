#pragma once
struct RwRaster;
struct RwImage;
#include <stdint.h>
namespace rh::rw::engine {
class RwImageFindRasterFormatCmd
{
public:
    RwImageFindRasterFormatCmd(RwRaster *raster, RwImage *image, uint32_t flags);
    bool Execute();

private:
    RwRaster *m_pRaster = nullptr;
    RwImage *m_pImage = nullptr;
    uint32_t m_nFlags = 0;
};
} // namespace rw_rh_engine
