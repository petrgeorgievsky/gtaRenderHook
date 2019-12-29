#include "rastersetimagecmd.h"
#include <Engine/Common/types/image_buffer_format.h>
#include <Engine/Common/types/image_buffer_info.h>
#include <Engine/Common/types/image_buffer_type.h>
#include <Engine/IRenderer.h>
#include <common_headers.h>
#include <rw_engine/global_definitions.h>
#include <rw_engine/rw_image/rw_image_funcs.h>
#include <rw_engine/rw_raster/rw_raster_macros_wrappers.h>

using namespace rh::rw::engine;

RwRasterSetImageCmd::RwRasterSetImageCmd( RwRaster *raster, RwImage *image )
    : m_pRaster( raster )
    , m_pImage( image )
{}

bool RwRasterSetImageCmd::Execute()
{
    RwUInt32 rasFormat;

    /* Normal type rasters are created with alpha by default */
    if ( m_pRaster->cType == rwRASTERTYPENORMAL ) {
        rasFormat = InternalImageFindFormat( m_pImage );
    } else {
        rasFormat = static_cast<RwUInt32>( RwRasterGetFormatMacro( m_pRaster ) );
    }
    // TODO: Implement dis
    void *&internalRaster = GetInternalRaster( m_pRaster );

    size_t pixels_size = static_cast<size_t>( m_pImage->height * m_pImage->width )
                         * sizeof( RwUInt32 );

    auto *pixels = static_cast<RwUInt32 *>( malloc( pixels_size ) );

    memset( pixels, 0x000000FF, pixels_size );

    for ( size_t y = 0; y < static_cast<size_t>( m_pImage->height ); y++ ) {
        for ( size_t x = 0; x < static_cast<size_t>( m_pImage->width ); x++ ) {
            auto *srcPixel = reinterpret_cast<RwRGBA *>(
                m_pImage->cpPixels + static_cast<size_t>( m_pImage->stride ) * y
                + x * sizeof( RwRGBA ) );
            pixels[y * static_cast<size_t>( m_pImage->width ) + x]
                = ( static_cast<RwUInt32>( srcPixel->alpha ) << 24 )
                  | ( static_cast<RwUInt32>( srcPixel->red ) << 16 )
                  | ( static_cast<RwUInt32>( srcPixel->green ) << 8 )
                  | ( static_cast<RwUInt32>( srcPixel->blue ) );
        }
    }

    rh::engine::ImageBufferInfo create_info{};
    create_info.width = static_cast<uint32_t>( m_pImage->width );
    create_info.height = static_cast<uint32_t>( m_pImage->height );
    create_info.mipLevels = 1;
    create_info.format = rh::engine::ImageBufferFormat::BGRA8;
    create_info.initialDataVec = {{static_cast<RwUInt32>( m_pImage->stride ), pixels}};
    create_info.type = rh::engine::ImageBufferType::TextureBuffer;

    internalRaster = rh::engine::g_pRHRenderer->AllocateImageBuffer( create_info );
    return true;
}
