#include "rasterlockcmd.h"
#include <common_headers.h>
using namespace rh::rw::engine;

RwRasterLockCmd::RwRasterLockCmd( RwRaster *raster, int32_t accessMode )
    : m_pRaster( raster ), m_nAccessMode( accessMode )
{
}

bool RwRasterLockCmd::Execute( void *&res_data_ptr )
{
    if ( m_pRaster == nullptr )
        return false;
    /* Prepare lock info */
    auto mipLevel = static_cast<uint8_t>( ( m_nAccessMode & 0xFF00 ) >> 8 );

    /* Pixels */
    m_pRaster->cpPixels = nullptr; //(RwUInt8 *)rasExt->lockedRect.pBits;

    /* Cache original width, height & stride */
    m_pRaster->originalWidth  = m_pRaster->width;
    m_pRaster->originalHeight = m_pRaster->height;

    /* Mip level width, height & stride */
    m_pRaster->width  = m_pRaster->width >> mipLevel;
    m_pRaster->height = m_pRaster->height >> mipLevel;

    m_pRaster->cpPixels = static_cast<uint8_t *>( malloc(
        static_cast<size_t>( m_pRaster->width * m_pRaster->height * 4 ) ) );

    /* Clamp width and height to 1 */
    if ( m_pRaster->width == 0 )
    {
        m_pRaster->width = 1;
    }

    if ( m_pRaster->height == 0 )
    {
        m_pRaster->height = 1;
    }

    /* Set the stride */
    m_pRaster->stride = 4 * m_pRaster->width; // rasExt->lockedRect.Pitch;
    res_data_ptr      = m_pRaster->cpPixels;

    return true;
}
