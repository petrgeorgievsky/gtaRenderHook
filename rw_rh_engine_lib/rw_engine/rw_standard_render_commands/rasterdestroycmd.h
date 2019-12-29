#pragma once
struct RwRaster;
namespace rh::rw::engine {
class RwRasterDestroyCmd
{
public:
    RwRasterDestroyCmd(RwRaster *raster);
    bool Execute();

private:
    RwRaster *m_pRaster;
};
} // namespace rw_rh_engine
