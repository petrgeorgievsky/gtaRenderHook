#include "nativetexturereadcmd.h"
#include "../global_definitions.h"
#include "../rh_backend/raster_backend.h"
#include "../rw_macro_constexpr.h"
#include "../system_funcs/rw_device_system_globals.h"
#include <DebugUtils/DebugLogger.h>
#include <Engine/Common/IDeviceState.h>
#include <Engine/Common/types/image_buffer_format.h>
#include <Engine/Common/types/image_buffer_info.h>
#include <Engine/Common/types/image_buffer_type.h>
#include <algorithm>
#include <common_headers.h>
#include <d3d9.h>
#include <ostream>
#include <queue>
#include <rw_engine/rh_backend/im2d_backend.h>
#include <rw_engine/rw_api_injectors.h>
#include <rw_engine/rw_rh_convert_funcs.h>
#include <rw_engine/system_funcs/load_texture_cmd.h>
#include <span>
#include <sstream>

using namespace rh::rw::engine;

struct rwNativeTexture
{
    friend std::ostream &operator<<( std::ostream &         os,
                                     const rwNativeTexture &texture )
    {
        os << "NativeTexture data block:\n\tid: " << texture.id
           << "\n\tfilterAndAddress: " << texture.filterAndAddress
           << "\n\tname: " << texture.name << "\n\tmask: " << texture.mask;
        return os;
    }
    int32_t id;               /* RwPlatformID,(rwID_D3D9) defined in batype.h */
    int32_t filterAndAddress; /* Same as in babintex.c */
    char    name[rwTEXTUREBASENAMELENGTH]; /* Texture name */
    char    mask[rwTEXTUREBASENAMELENGTH]; /* Texture mask name */
};

struct rwD3D9NativeRaster
{
    friend std::ostream &operator<<( std::ostream &            os,
                                     const rwD3D9NativeRaster &raster )
    {
        os << "D3D9NativeRaster data block:\n\tformat: " << raster.format
           << " d3dFormat: " << raster.d3dFormat << " width: " << raster.width
           << " height: " << raster.height
           << " depth: " << (uint32_t)raster.depth
           << " numMipLevels: " << (uint32_t)raster.numMipLevels
           << " type: " << (uint32_t)raster.type
           << " flags: " << (uint32_t)raster.flags;
        return os;
    }
    uint32_t  format;       /* Raster format flags */
    D3DFORMAT d3dFormat;    /* D3D pixel format */
    uint16_t  width;        /* Raster width */
    uint16_t  height;       /* Raster height */
    uint8_t   depth;        /* Raster depth */
    uint8_t   numMipLevels; /* The number of mip levels to load */
    uint8_t   type;         /* The raster type */
    uint8_t flags; /* This raster has an alpha component, automipmapgen, etc */
};

struct rwD3D8NativeRaster
{
    friend std::ostream &operator<<( std::ostream &            os,
                                     const rwD3D8NativeRaster &raster )
    {
        os << "D3D8NativeRaster data block:\n\tformat: " << raster.format
           << " alpha: " << raster.alpha << " width: " << raster.width
           << " height: " << raster.height
           << " depth: " << (uint32_t)raster.depth
           << " numMipLevels: " << (uint32_t)raster.numMipLevels
           << " type: " << (uint32_t)raster.type
           << " dxtFormat: " << (uint32_t)raster.dxtFormat;
        return os;
    }
    int32_t  format;       /* Raster format flags */
    int32_t  alpha;        /* This raster has an alpha component */
    uint16_t width;        /* Raster width */
    uint16_t height;       /* Raster height */
    uint8_t  depth;        /* Raster depth */
    uint8_t  numMipLevels; /* The number of mip levels to load */
    uint8_t  type;         /* The raster type */
    uint8_t  dxtFormat;    /* 1-5 for DXT format 0 for normal */
};

union rwD3DNativeRaster
{
    rwD3D8NativeRaster d3d8_;
    rwD3D9NativeRaster d3d9_;
};

RwNativeTextureReadCmd::RwNativeTextureReadCmd( RwStream *  stream,
                                                RwTexture **texture )
    : m_pStream( stream ), m_pTexture( texture )
{
}

bool RwNativeTextureReadCmd::Execute()
{
    RwRaster *raster = nullptr;

    uint32_t          length, version;
    rwD3DNativeRaster nativeRaster{};
    rwNativeTexture   nativeTexture{};

    auto timestamp = std::chrono::high_resolution_clock::now();

    if ( !g_pIO_API.fpFindChunk( m_pStream, rwID_STRUCT, &length, &version ) )
        return false;
    g_pIO_API.fpRead( m_pStream, reinterpret_cast<void *>( &nativeTexture ),
                      sizeof( rwNativeTexture ) );

    std::stringstream debug_output;
    debug_output << nativeTexture;

    g_pIO_API.fpRead( m_pStream, reinterpret_cast<void *>( &nativeRaster ),
                      sizeof( rwD3DNativeRaster ) );

    bool compressed = false, isCubemap = false;

    switch ( nativeTexture.id )
    {
    case rwID_PCD3D8:
        debug_output << nativeRaster.d3d8_;
        compressed = nativeRaster.d3d8_.dxtFormat != 0;
        break;
    case rwID_PCD3D9:
        debug_output << nativeRaster.d3d9_;
        compressed =
            static_cast<bool>( nativeRaster.d3d9_.flags & ( 1u << 3u ) );
        isCubemap =
            static_cast<bool>( nativeRaster.d3d9_.flags & ( 1u << 1u ) );
        break;
    }

    std::string debug_output_str = debug_output.str();

    debug::DebugLogger::Log( debug_output_str );

    if ( compressed ) // is compressed
    {
        debug::DebugLogger::Log( "compressed raster path" );
        // Validate format
        // ...
        // Create a raster
        raster = g_pRaster_API.fpCreateRaster(
            nativeRaster.d3d9_.width, nativeRaster.d3d9_.height,
            static_cast<int32_t>( nativeRaster.d3d9_.depth ),
            static_cast<int32_t>( nativeRaster.d3d9_.type |
                                  nativeRaster.d3d9_.format |
                                  rwRASTERDONTALLOCATE ) );
        // Attach API texture
        if ( isCubemap ) // is cubemap
        {
            debug::DebugLogger::Log( "cubemap raster path" );
            // Create a raster
            // ...
            // Attach API texture
        }
    }
    else if ( isCubemap ) // is cubemap
    {
        debug::DebugLogger::Log( "cubemap raster path" );
        // Create a raster
        // ...
        // Attach API texture
    }
    else if ( nativeRaster.d3d9_.format &
              static_cast<uint32_t>(
                  ~( rwRASTERFORMATAUTOMIPMAP | rwRASTERFORMATMIPMAP ) ) )
    {
        debug::DebugLogger::Log( "rw raster path" );
        // Create a raster
        // ...
        raster = g_pRaster_API.fpCreateRaster(
            nativeRaster.d3d9_.width, nativeRaster.d3d9_.height,
            static_cast<int32_t>( nativeRaster.d3d9_.depth ),
            static_cast<int32_t>( nativeRaster.d3d9_.type |
                                  nativeRaster.d3d9_.format |
                                  rwRASTERDONTALLOCATE ) );
    }
    else
    {
        debug::DebugLogger::Log( "weird raster path" );
    }

    if ( raster == nullptr )
        return false;

    rh::engine::ImageBufferFormat rhFormat =
        RwNativeFormatToRHImageBufferFormat(
            nativeRaster.d3d9_.d3dFormat,
            static_cast<RwRasterFormat>( nativeRaster.d3d9_.format ),
            static_cast<RwPlatformID>( nativeTexture.id ),
            nativeRaster.d3d8_.dxtFormat );

    uint32_t bytesPerBlock, blockSize = 4;

    std::vector<RwRGBA> palette;

    /* Load the palette if palletized */
    if ( nativeRaster.d3d9_.format & rwRASTERFORMATPAL4 )
    {
        uint32_t size = sizeof( RwRGBA ) * 32;

        palette.resize( size / sizeof( RwRGBA ) );

        if ( g_pIO_API.fpRead( m_pStream,
                               reinterpret_cast<void *>( palette.data() ),
                               size ) != size )
        {
            rh::debug::DebugLogger::Error(
                "Failed to read 4bit palette data!" );

            g_pRaster_API.fpDestroyRaster( raster );

            return false;
        }
        rhFormat = rh::engine::ImageBufferFormat::BGRA8;
    }
    else if ( nativeRaster.d3d9_.format & rwRASTERFORMATPAL8 )
    {
        constexpr uint32_t size = sizeof( RwRGBA ) * 256;

        palette.resize( size / sizeof( RwRGBA ) );

        if ( g_pIO_API.fpRead( m_pStream,
                               reinterpret_cast<void *>( palette.data() ),
                               size ) != size )
        {
            rh::debug::DebugLogger::Error(
                "Failed to read 8bit palette data!" );

            g_pRaster_API.fpDestroyRaster( raster );

            return false;
        }
        rhFormat = rh::engine::ImageBufferFormat::BGRA8;
    }

    switch ( rhFormat )
    {
    case rh::engine::ImageBufferFormat::A8: bytesPerBlock = 4; break;
    case rh::engine::ImageBufferFormat::BGRA4:
    case rh::engine::ImageBufferFormat::BGR5A1:
    case rh::engine::ImageBufferFormat::B5G6R5:
    case rh::engine::ImageBufferFormat::BC1:
    case rh::engine::ImageBufferFormat::BC4: bytesPerBlock = 8; break;
    default: bytesPerBlock = 16; break;
    }
    auto *internalRaster = GetBackendRasterExt( raster );
    debug_output.str( "" );
    debug_output.clear();
    debug_output << "Texture adress: 0x" << std::hex
                 << reinterpret_cast<INT_PTR>( internalRaster ) << '\n';
    rh::debug::DebugLogger::Log( debug_output.str() );

    auto convert_paletted_mip_level =
        [&]( MipLevelHeader &mip_header, uint32_t width, uint32_t height,
             uint32_t size, uint8_t *raw_pixels, uint32_t *result_data,
             bool has_alpha ) {
            // Convert paletted raster if needed
            auto result_size = height * width * sizeof( uint32_t );

            if ( !has_alpha )
                for ( size_t j = 0; j < size; j++ )
                    result_data[j] = rwRGBA::Long_RGB( palette[raw_pixels[j]] );
            else
                for ( size_t j = 0; j < size; j++ )
                    result_data[j] = rwRGBA::Long( palette[raw_pixels[j]] );

            mip_header.mSize   = result_size;
            mip_header.mStride = bytesPerBlock * ( ( width + 3 ) / blockSize );
        };

    // Some txd records can have invalid mip level counter, default it
    // to 1 for now
    const uint32_t numMipLevels = max( nativeRaster.d3d9_.numMipLevels, 1u );
    const bool     convert_from_pal =
        nativeRaster.d3d9_.format & rwRASTERFORMATPAL4 ||
        nativeRaster.d3d9_.format & rwRASTERFORMATPAL8;
    const bool has_alpha =
        static_cast<bool>( ( nativeTexture.id == rwID_PCD3D8 )
                               ? nativeRaster.d3d8_.alpha
                               : nativeRaster.d3d9_.flags & ( 1u << 0u ) );

    RasterHeader header{ .mWidth         = nativeRaster.d3d9_.width,
                         .mHeight        = nativeRaster.d3d9_.height,
                         .mDepth         = 1,
                         .mFormat        = static_cast<uint32_t>( rhFormat ),
                         .mMipLevelCount = numMipLevels };

    LoadTextureCmdImpl load_texture_cmd( gRenderClient->GetTaskQueue() );

    uint32_t mip_width  = nativeRaster.d3d9_.width;
    uint32_t mip_height = nativeRaster.d3d9_.height;

    internalRaster->mImageId = load_texture_cmd.Invoke(
        header, [&]( MemoryWriter &writer, MipLevelHeader &mip_header ) {
            writer.Skip( sizeof( MipLevelHeader ) );

            auto *image_memory = writer.CurrentPtr<uint32_t>();
            if ( convert_from_pal )
                writer.Skip( mip_height * mip_width * sizeof( uint32_t ) );
            auto *pixels = writer.CurrentPtr<uint8_t>();
            if ( convert_from_pal )
                writer.SeekFromCurrent(
                    -static_cast<int64_t>( mip_height * mip_width ) *
                    sizeof( uint32_t ) );

            uint32_t size;
            g_pIO_API.fpRead( m_pStream, reinterpret_cast<char *>( &size ),
                              sizeof( size ) );
            g_pIO_API.fpRead( m_pStream, reinterpret_cast<char *>( pixels ),
                              size );

            if ( convert_from_pal )
                convert_paletted_mip_level( mip_header, mip_width, mip_height,
                                            size, pixels, image_memory,
                                            has_alpha );
            else
            {
                // Fix rgb8 format "alpha" channel
                if ( !compressed && ( nativeRaster.d3d9_.format &
                                      rwRASTERFORMAT888 ) == rwRASTERFORMAT888 )
                {
                    std::span<RwRGBA> rgba_pixels(
                        reinterpret_cast<RwRGBA *>( pixels ), size / 4 );
                    for ( auto &pix : rgba_pixels )
                        pix.alpha = 0xff;
                }

                mip_header.mSize = size;
                mip_header.mStride =
                    bytesPerBlock * ( ( mip_width + 3 ) / blockSize );
            }
            writer.Skip( mip_header.mSize );

            mip_width  = ( std::max )( mip_width >> 1u, 1u );
            mip_height = ( std::max )( mip_height >> 1u, 1u );
            return true;
        } );

    RwTexture *texture = g_pTexture_API.fpCreateTexture( raster );
    if ( texture == nullptr )
        return false;
    rwTexture::SetFilterMode(
        texture, ( (uint32_t)nativeTexture.filterAndAddress ) & 0xFFu );
    rwTexture::SetAddressingU(
        texture, ( (uint32_t)nativeTexture.filterAndAddress >> 8u ) & 0x0Fu );
    rwTexture::SetAddressingV(
        texture, ( (uint32_t)nativeTexture.filterAndAddress >> 12u ) & 0x0Fu );
    if ( g_pTexture_API.fpTextureSetName )
        g_pTexture_API.fpTextureSetName( texture, nativeTexture.name );
    if ( g_pTexture_API.fpTextureSetMaskName )
        g_pTexture_API.fpTextureSetMaskName( texture, nativeTexture.mask );
    *m_pTexture = texture;
    debug_output.clear();
    debug_output << "Texture loading : " << std::dec
                 << std::chrono::duration_cast<std::chrono::microseconds>(
                        std::chrono::high_resolution_clock::now() - timestamp )
                        .count()
                 << " mcs.";
    debug::DebugLogger::Log( debug_output.str() );
    return true;
}
