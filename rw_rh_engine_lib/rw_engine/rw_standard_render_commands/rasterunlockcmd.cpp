#include "rasterunlockcmd.h"
#include <common_headers.h>
using namespace rh::rw::engine;
RwRasterUnlockCmd::RwRasterUnlockCmd( RwRaster *raster )
    : m_pRaster( raster )
{}

bool RwRasterUnlockCmd::Execute()
{
    /* Restore the original width, height & stride */
    m_pRaster->width = m_pRaster->originalWidth;
    m_pRaster->height = m_pRaster->originalHeight;

    m_pRaster->stride = 0;
    free( m_pRaster->cpPixels );
    m_pRaster->cpPixels = nullptr;
    return true;
}
