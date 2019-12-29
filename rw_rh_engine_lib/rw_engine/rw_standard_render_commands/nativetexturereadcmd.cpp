#include "nativetexturereadcmd.h"
#include "../global_definitions.h"
#include "../rw_macro_constexpr.h"
#include <DebugUtils/DebugLogger.h>
#include <Engine/Common/types/image_buffer_format.h>
#include <Engine/Common/types/image_buffer_info.h>
#include <Engine/Common/types/image_buffer_type.h>
#include <Engine/IRenderer.h>
#include <common_headers.h>
#include <d3d9.h>
#include <rw_engine/rw_api_injectors.h>
#include <rw_engine/rw_rh_convert_funcs.h>
#include <sstream>

using namespace rh::rw::engine;

struct _rwNativeTexture
{
    RwInt32 id;               /* RwPlatformID,(rwID_D3D9) defined in batype.h */
    RwInt32 filterAndAddress; /* Same as in babintex.c */
    RwChar  name[rwTEXTUREBASENAMELENGTH]; /* Texture name */
    RwChar  mask[rwTEXTUREBASENAMELENGTH]; /* Texture mask name */
};

struct _rwD3D9NativeRaster
{
    RwUInt32  format;       /* Raster format flags */
    D3DFORMAT d3dFormat;    /* D3D pixel format */
    RwUInt16  width;        /* Raster width */
    RwUInt16  height;       /* Raster height */
    RwUInt8   depth;        /* Raster depth */
    RwUInt8   numMipLevels; /* The number of mip levels to load */
    RwUInt8   type;         /* The raster type */
    RwUInt8 flags; /* This raster has an alpha component, automipmapgen, etc */
};

struct _rwD3D8NativeRaster
{
    RwInt32  format;       /* Raster format flags */
    RwBool   alpha;        /* This raster has an alpha component */
    RwUInt16 width;        /* Raster width */
    RwUInt16 height;       /* Raster height */
    RwUInt8  depth;        /* Raster depth */
    RwUInt8  numMipLevels; /* The number of mip levels to load */
    RwUInt8  type;         /* The raster type */
    RwUInt8  dxtFormat;    /* 1-5 for DXT format 0 for normal */
};

union _rwD3DNativeRaster {
    _rwD3D8NativeRaster d3d8_;
    _rwD3D9NativeRaster d3d9_;
};

RwNativeTextureReadCmd::RwNativeTextureReadCmd( RwStream *  stream,
                                                RwTexture **texture )
    : m_pStream( stream ), m_pTexture( texture )
{
}

bool RwNativeTextureReadCmd::Execute()
{
    RwRaster *raster = nullptr;

    uint32_t           length, version;
    _rwD3DNativeRaster nativeRaster{};
    _rwNativeTexture   nativeTexture{};

    if ( !g_pIO_API.fpFindChunk( m_pStream, rwID_STRUCT, &length, &version ) )
        return false;
    g_pIO_API.fpRead( m_pStream, reinterpret_cast<void *>( &nativeTexture ),
                      sizeof( _rwNativeTexture ) );

    std::stringstream debug_output;
    debug_output << "NativeTexture data block:\n\tid:" << nativeTexture.id
                 << "\n\tfilterAndAddress:" << nativeTexture.filterAndAddress
                 << "\n\tname:" << nativeTexture.name
                 << "\n\tmask:" << nativeTexture.mask << std::endl;

    g_pIO_API.fpRead( m_pStream, reinterpret_cast<void *>( &nativeRaster ),
                      sizeof( _rwD3DNativeRaster ) );

    bool compressed = false, isCubemap = false;

    switch ( nativeTexture.id )
    {
    case rwID_PCD3D8:
        debug_output << "D3D8NativeRaster data block:\n\tdxtFormat:"
                     << nativeRaster.d3d8_.dxtFormat << "\n\tdepth:"
                     << static_cast<uint32_t>( nativeRaster.d3d8_.depth )
                     << "\n\tformat:" << nativeRaster.d3d8_.format
                     << "\n\twidth:" << nativeRaster.d3d8_.width
                     << "\n\theight:" << nativeRaster.d3d8_.height
                     << "\n\tnumMipLevels:"
                     << static_cast<uint32_t>( nativeRaster.d3d8_.numMipLevels )
                     << "\n\ttype:"
                     << static_cast<uint32_t>( nativeRaster.d3d8_.type )
                     << "\n\talpha:"
                     << static_cast<uint32_t>( nativeRaster.d3d8_.alpha )
                     << '\n';
        compressed = nativeRaster.d3d8_.dxtFormat != 0;
        break;
    case rwID_PCD3D9:
        debug_output << "D3D9NativeRaster data block:\n\td3dFormat:"
                     << nativeRaster.d3d9_.d3dFormat << "\n\tdepth:"
                     << static_cast<uint32_t>( nativeRaster.d3d9_.depth )
                     << "\n\tformat:" << nativeRaster.d3d9_.format
                     << "\n\twidth:" << nativeRaster.d3d9_.width
                     << "\n\theight:" << nativeRaster.d3d9_.height
                     << "\n\tnumMipLevels:"
                     << static_cast<uint32_t>( nativeRaster.d3d9_.numMipLevels )
                     << "\n\ttype:"
                     << static_cast<uint32_t>( nativeRaster.d3d9_.type )
                     << '\n';
        compressed = nativeRaster.d3d9_.flags & ( 1 << 3 );
        isCubemap  = nativeRaster.d3d9_.flags & ( 1 << 1 );
        break;
    }

    std::string debug_output_str = debug_output.str();

    rh::debug::DebugLogger::Log( debug_output_str );

    if ( compressed ) // is compressed
    {
        rh::debug::DebugLogger::Log( "compressed raster path" );
        // Validate format
        // ...
        // Create a raster
        raster = g_pRaster_API.fpCreateRaster(
            nativeRaster.d3d9_.width, nativeRaster.d3d9_.height,
            static_cast<RwInt32>( nativeRaster.d3d9_.depth ),
            static_cast<RwInt32>( nativeRaster.d3d9_.type |
                                  nativeRaster.d3d9_.format |
                                  rwRASTERDONTALLOCATE ) );
        // Attach API texture
        if ( isCubemap ) // is cubemap
        {
            rh::debug::DebugLogger::Log( "cubemap raster path" );
            // Create a raster
            // ...
            // Attach API texture
        }
    }
    else if ( isCubemap ) // is cubemap
    {
        rh::debug::DebugLogger::Log( "cubemap raster path" );
        // Create a raster
        // ...
        // Attach API texture
    }
    else if ( nativeRaster.d3d9_.format &
              static_cast<RwUInt32>(
                  ~( rwRASTERFORMATAUTOMIPMAP | rwRASTERFORMATMIPMAP ) ) )
    {
        rh::debug::DebugLogger::Log( "rw raster path" );
        // Create a raster
        // ...
        raster = g_pRaster_API.fpCreateRaster(
            nativeRaster.d3d9_.width, nativeRaster.d3d9_.height,
            static_cast<RwInt32>( nativeRaster.d3d9_.depth ),
            static_cast<RwInt32>( nativeRaster.d3d9_.type |
                                  nativeRaster.d3d9_.format |
                                  rwRASTERDONTALLOCATE ) );
    }
    else
    {
        rh::debug::DebugLogger::Log( "bullshit raster path" );
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
        RwUInt32 size;

        size = sizeof( RwRGBA ) * 32;

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
        RwUInt32 size;

        size = sizeof( RwRGBA ) * 256;

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

    std::vector<rh::engine::ImageBufferRawData> initial_data_vec(
        nativeRaster.d3d9_.numMipLevels );
    std::vector<RwUInt8 *> data_ptrs;
    {
        RwUInt32 /*autoMipmap, face,*/ numMipLevels;
        numMipLevels = nativeRaster.d3d9_.numMipLevels;

        for ( RwUInt32 i = 0; i < numMipLevels; i++ )
        {
            RwUInt32 size, height = max( nativeRaster.d3d9_.height >> i, 1 ),
                           width = max( nativeRaster.d3d9_.width >> i, 1 );
            RwUInt8 * pixels;
            RwUInt32 *converted_pixels;

            g_pIO_API.fpRead( m_pStream, reinterpret_cast<char *>( &size ),
                              sizeof( size ) );

            pixels = static_cast<RwUInt8 *>( malloc( size ) );

            g_pIO_API.fpRead( m_pStream, reinterpret_cast<char *>( pixels ),
                              size );

            // Convert paletted raster if needed
            if ( nativeRaster.d3d9_.format & rwRASTERFORMATPAL4 ||
                 nativeRaster.d3d9_.format & rwRASTERFORMATPAL8 )
            {
                converted_pixels = static_cast<RwUInt32 *>(
                    malloc( height * width * sizeof( RwUInt32 ) ) );
                bool has_alpha = ( nativeTexture.id == rwID_PCD3D8 )
                                     ? nativeRaster.d3d8_.alpha
                                     : nativeRaster.d3d9_.flags & ( 1 << 0 );
                for ( size_t j = 0; j < size; j++ )
                {
                    auto color = palette[pixels[j]];
                    converted_pixels[j] =
                        rwRGBA::Long( color.red, color.green, color.blue,
                                      has_alpha ? color.alpha : 255 );
                }
                free( pixels );
                pixels = reinterpret_cast<RwUInt8 *>( converted_pixels );
            }
            data_ptrs.push_back( pixels );
            initial_data_vec[i] = {
                bytesPerBlock * ( ( width + 3 ) / blockSize ), pixels};
        }
    }

    void *&internalRaster = GetInternalRaster( raster );

    rh::engine::ImageBufferInfo create_info{};
    create_info.width          = nativeRaster.d3d9_.width;
    create_info.height         = nativeRaster.d3d9_.height;
    create_info.mipLevels      = nativeRaster.d3d9_.numMipLevels;
    create_info.format         = rhFormat;
    create_info.initialDataVec = initial_data_vec;
    create_info.type           = rh::engine::ImageBufferType::TextureBuffer;

    internalRaster =
        rh::engine::g_pRHRenderer->AllocateImageBuffer( create_info );
    debug_output.str( "" );
    debug_output.clear();
    debug_output << "Texture adress: 0x" << std::hex
                 << reinterpret_cast<INT_PTR>( internalRaster ) << '\n';
    rh::debug::DebugLogger::Log( debug_output.str() );
    internalRaster = GetInternalRaster( raster );
    for ( size_t i = 0; i < data_ptrs.size(); i++ )
    {
        free( data_ptrs[i] );
        data_ptrs[i] = nullptr;
    }
    RwTexture *texture = g_pTexture_API.fpCreateTexture( raster );
    if ( texture == nullptr )
        return false;
    rwTexture::SetFilterMode( texture, nativeTexture.filterAndAddress & 0xFF );
    rwTexture::SetAddressingU( texture,
                               ( nativeTexture.filterAndAddress >> 8 ) & 0x0F );
    rwTexture::SetAddressingV(
        texture, ( nativeTexture.filterAndAddress >> 12 ) & 0x0F );
    if ( g_pTexture_API.fpTextureSetName )
        g_pTexture_API.fpTextureSetName( texture, nativeTexture.name );
    if ( g_pTexture_API.fpTextureSetMaskName )
        g_pTexture_API.fpTextureSetMaskName( texture, nativeTexture.mask );
    *m_pTexture = texture;
    return true;
}
