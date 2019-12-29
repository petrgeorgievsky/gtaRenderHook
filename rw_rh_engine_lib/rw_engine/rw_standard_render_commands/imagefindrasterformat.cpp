#include "imagefindrasterformat.h"
#include <common_headers.h>
#include <rw_engine/rw_image/rw_image_funcs.h>
#include <rw_engine/rw_rh_convert_funcs.h>

using namespace rh::rw::engine;
RwImageFindRasterFormatCmd::RwImageFindRasterFormatCmd( RwRaster *raster,
                                                        RwImage *image,
                                                        uint32_t flags )
    : m_pRaster( raster )
    , m_pImage( image )
    , m_nFlags( flags )
{}

bool RwImageFindRasterFormatCmd::Execute()
{
    m_pRaster->width = m_pImage->width;
    m_pRaster->height = m_pImage->height;
    m_pRaster->depth = 0;
    m_pRaster->cType = m_nFlags & rwRASTERTYPEMASK;

    /* Find the rasters format */
    switch ( m_nFlags & rwRASTERTYPEMASK ) {
    case rwRASTERTYPENORMAL: {
        RwUInt32 format;

        /* Check size */
        //_rwD3D9CheckRasterSize( &( ras->width ), &( ras->height ), flags );

        /* Find the image format */
        format = InternalImageFindFormat( m_pImage );

        /* rwRASTERTYPENORMAL - 'Linear' textures can not have mip maps */
        // RWASSERT( !( ( rwRASTERFORMATMIPMAP | rwRASTERFORMATAUTOMIPMAP ) & flags
        // ) );

        /* Only Mipmap if actually requested */
        format |= m_nFlags
                  & static_cast<RwUInt32>( ~( rwRASTERFORMATMIPMAP | rwRASTERFORMATAUTOMIPMAP ) );

        m_pRaster->cFormat = static_cast<RwUInt8>( format >> 8 );
        /* Check format */
        // RWRETURN( _rwD3D9CheckRasterFormat( raster, format | ( flags &
        // rwRASTERTYPEMASK ) ) );
    } break;
    case rwRASTERTYPETEXTURE:
    case rwRASTERTYPECAMERATEXTURE: {
        RwUInt32 format;

        /* Check size */
        //_rwD3D9CheckRasterSize( &( ras->width ), &( ras->height ), flags );

        /* Find the image format */
        format = InternalImageFindFormat( m_pImage );

        /* Only Mipmap if actually requested */
        format |= m_nFlags & ( rwRASTERFORMATMIPMAP | rwRASTERFORMATAUTOMIPMAP );

        m_pRaster->cFormat = static_cast<RwUInt8>( format >> 8 );
        /* Check format */
        // RWRETURN( _rwD3D9CheckRasterFormat( raster, format | ( flags &
        // rwRASTERTYPEMASK ) ) );
    } break;
    case rwRASTERTYPECAMERA:
    case rwRASTERTYPEZBUFFER: {
        /* Just take the default case */
        // RWRETURN( _rwD3D9CheckRasterFormat( raster, flags ) );
    } break;
    default: {
        /* Don't know what one of those is... */
        // RWERROR( ( E_RW_INVRASTERFORMAT ) );
    }
    }

    return true;
}
