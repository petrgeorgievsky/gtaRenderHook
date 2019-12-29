#pragma once
#include <cstdint>
struct RwRaster;
namespace rh::rw::engine {
class RwRasterLockCmd
{
public:
    RwRasterLockCmd( RwRaster *raster, int32_t accessMode );
    bool Execute( void *&res_data_ptr );

private:
    RwRaster *m_pRaster = nullptr;
    int32_t m_nAccessMode = 0;
};
} // namespace rw_rh_engine
