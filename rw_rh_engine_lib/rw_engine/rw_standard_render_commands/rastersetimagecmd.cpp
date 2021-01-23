#include "rastersetimagecmd.h"
#include <Engine/Common/types/image_buffer_format.h>
#include <Engine/Common/types/image_buffer_info.h>
#include <Engine/Common/types/image_buffer_type.h>
#include <Engine/IRenderer.h>
#include <common_headers.h>
#include <rw_engine/global_definitions.h>
#include <rw_engine/rh_backend/raster_backend.h>
#include <rw_engine/rw_image/rw_image_funcs.h>
#include <rw_engine/rw_macro_constexpr.h>
#include <rw_engine/rw_raster/rw_raster_macros_wrappers.h>
#include <rw_engine/system_funcs/rw_device_system_globals.h>

using namespace rh::rw::engine;

rh::rw::engine::RwRasterSetImageCmd::RwRasterSetImageCmd( RwRaster *raster,
                                                          RwImage * image )
    : m_pRaster( raster ), m_pImage( image )
{
}

bool rh::rw::engine::RwRasterSetImageCmd::Execute()
{
    uint32_t rasFormat;

    /* Normal type rasters are created with alpha by default */
    if ( m_pRaster->cType == rwRASTERTYPENORMAL )
    {
        rasFormat = InternalImageFindFormat( m_pImage );
    }
    else
    {
        rasFormat =
            static_cast<uint32_t>( RwRasterGetFormatMacro( m_pRaster ) );
    }
    // TODO: Implement dis

    size_t pixels_size =
        static_cast<size_t>( m_pImage->height * m_pImage->width ) *
        sizeof( uint32_t );

    auto *pixels = static_cast<uint8_t *>( malloc( pixels_size ) );
    std::vector<uint32_t> pixel_data( m_pImage->height * m_pImage->width, 0 );

    if ( m_pImage->depth == 4 || m_pImage->depth == 8 )
    {
        std::array<uint32_t, 256> pal{};
        if ( ( rasFormat & rwRASTERFORMAT888 ) == rwRASTERFORMAT888 )
        {
            for ( auto x = 0; x < ( 1u << (uint32_t)m_pImage->depth ); x++ )
            {
                const RwRGBA &pixIn = m_pImage->palette[x];

                pal[x] = ( 0xFF000000u ) | ( (uint32_t)pixIn.red << 16u ) |
                         ( (uint32_t)pixIn.green << 8u ) |
                         ( (uint32_t)pixIn.blue );
            }
        }
        else
        {
            for ( auto x = 0; x < ( 1u << (uint32_t)m_pImage->depth ); x++ )
            {
                const RwRGBA &pixIn = ( m_pImage->palette[x] );

                pal[x] = ( (uint32_t)pixIn.alpha << 24u ) |
                         ( (uint32_t)pixIn.red << 16u ) |
                         ( (uint32_t)pixIn.green << 8u ) |
                         ( (uint32_t)pixIn.blue );
            }
        }

        for ( auto y = 0; y < m_pImage->height; y++ )
        {
            auto line = m_pImage->stride * y;
            for ( auto x = 0; x < m_pImage->width; x++ )
            {
                pixel_data[line + x] = pal[m_pImage->cpPixels[line + x]];
            }
        }
    }
    else if ( 32 == m_pImage->depth )
    {
        auto *rgba_img = reinterpret_cast<RwRGBA *>( m_pImage->cpPixels );
        /* 32 bit image */
        if ( ( rasFormat & rwRASTERFORMAT888 ) == rwRASTERFORMAT888 )
        {
            for ( auto y = 0; y < m_pImage->height; y++ )
            {
                auto line = m_pImage->stride * y / sizeof( RwRGBA );
                for ( auto x = 0; x < m_pImage->width; x++ )
                {
                    pixel_data[line + x] =
                        ( (uint32_t)0xFF000000u ) |
                        ( (uint32_t)rgba_img[line + x].red << 16u ) |
                        ( (uint32_t)rgba_img[line + x].green << 8u ) |
                        ( (uint32_t)rgba_img[line + x].blue );
                }
            }
        }
        else if ( ( rasFormat & rwRASTERFORMAT8888 ) == rwRASTERFORMAT8888 )
        {
            for ( auto y = 0; y < m_pImage->height; y++ )
            {
                auto line = m_pImage->stride * y / sizeof( RwRGBA );

                for ( auto x = 0; x < m_pImage->width; x++ )
                {
                    pixel_data[line + x] =
                        ( (uint32_t)rgba_img[line + x].alpha << 24u ) |
                        ( (uint32_t)rgba_img[line + x].red << 16u ) |
                        ( (uint32_t)rgba_img[line + x].green << 8u ) |
                        ( (uint32_t)rgba_img[line + x].blue );
                }
            }
        }
    }

    auto *internalRaster = GetBackendRasterExt( m_pRaster );

    RasterHeader header{};
    header.mWidth         = m_pImage->width;
    header.mHeight        = m_pImage->height;
    header.mDepth         = 1;
    header.mMipLevelCount = 1;
    header.mFormat =
        static_cast<uint32_t>( rh::engine::ImageBufferFormat::BGRA8 );
    std::pair<MipLevelHeader, uint8_t *> mip_header{
        { static_cast<uint32_t>( pixels_size ),
          static_cast<uint32_t>( 16 * ( ( m_pImage->width + 3 ) / 4 ) ) },
        { reinterpret_cast<uint8_t *>( pixel_data.data() ) } };
    int64_t img_id = -1;
    gRenderClient->GetTaskQueue().ExecuteTask(
        SharedMemoryTaskType::TEXTURE_LOAD,
        [&header, &mip_header]( MemoryWriter &&writer ) {
            // serialize
            writer.Write( &header );
            writer.Write( &mip_header.first );
            writer.Write( mip_header.second, mip_header.first.mSize );
        },
        [&img_id]( MemoryReader &&memory_reader ) {
            // deserialize
            img_id = *memory_reader.Read<int64_t>();
        } );
    internalRaster->mImageId = img_id >= 0 ? img_id : 0xBADF00D;
    free( pixels );
    return true;
}
