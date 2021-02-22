#pragma once
struct RwRaster;
#include <cstdint>
namespace rh::rw::engine
{

class RwRasterShowRasterCmd
{
  public:
    RwRasterShowRasterCmd( RwRaster *raster, int32_t flags );
    bool Execute();

  private:
    [[maybe_unused]] RwRaster *m_pRaster = nullptr;
    [[maybe_unused]] int32_t   m_nFlags  = 0;
};

} // namespace rh::rw::engine
