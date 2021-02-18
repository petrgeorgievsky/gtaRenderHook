#include "rasterdestroycmd.h"

using namespace rh::rw::engine;
RwRasterDestroyCmd::RwRasterDestroyCmd( RwRaster *raster ) : m_pRaster( raster )
{
}

bool RwRasterDestroyCmd::Execute()
{
    if ( m_pRaster == nullptr )
        return false;
    return true;
}
