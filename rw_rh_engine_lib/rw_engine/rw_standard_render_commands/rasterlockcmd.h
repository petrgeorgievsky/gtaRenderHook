#pragma once
#include <cstdint>
struct RwRaster;
namespace rh::rw::engine
{
class RwRasterLockCmd
{
  public:
    RwRasterLockCmd( RwRaster *raster, int32_t accessMode );
    bool Execute( void *&res_data_ptr );

  private:
    RwRaster *mRaster     = nullptr;
    int32_t   mAccessMode = 0;
};
} // namespace rh::rw::engine
