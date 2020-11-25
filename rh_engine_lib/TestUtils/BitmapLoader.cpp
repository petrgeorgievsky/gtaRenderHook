//
// Created by peter on 02.08.2020.
//

#include "BitmapLoader.h"
#include "DebugUtils/DebugLogger.h"
#include "common.h"
#include <Engine/Common/IDeviceState.h>
#include <cassert>
#include <filesystem>

void rh::tests::LoadBMPImage( std::string_view       path,
                              engine::IDeviceState & device,
                              engine::IImageBuffer **res_buffer )
{
    assert( res_buffer != nullptr );

    auto fspath = std::filesystem::path( path );
    if ( !std::filesystem::exists( fspath ) )
    {
        std::stringstream ss;
        ss << "Trying to open file, that doesn't exist:" << fspath;
        debug::DebugLogger::Error( ss.str() );
    }

    std::ifstream file( ( fspath ), std::ios::binary | std::ios::in );

    BITMAPFILEHEADER header{};
    BITMAPINFOHEADER info_header{};

    file.read( reinterpret_cast<char *>( &header ), sizeof( header ) );
    file.read( reinterpret_cast<char *>( &info_header ),
               sizeof( info_header ) );

    auto tex_size =
        info_header.biBitCount / 8 * info_header.biWidth * info_header.biHeight;
    file.seekg( header.bfOffBits, std::ios::beg );

    std::string texture_buff( tex_size, 0 );

    file.read( texture_buff.data(), tex_size );
    assert( file );
    // convert to rgba
    std::string texture_buff_rgba( ( tex_size / 3 ) * 4, 0 );

    for ( auto idx = 0; idx < tex_size / 3; idx++ )
    {
        texture_buff_rgba[idx * 4]     = texture_buff[idx * 3];
        texture_buff_rgba[idx * 4 + 1] = texture_buff[idx * 3 + 1];
        texture_buff_rgba[idx * 4 + 2] = texture_buff[idx * 3 + 2];
        texture_buff_rgba[idx * 4 + 3] = 0xF;
    }

    engine::ImageBufferCreateParams image_buffer_ci{
        .mDimension = engine::ImageDimensions::d2D,
        .mFormat    = engine::ImageBufferFormat::RGBA8,
        .mHeight    = static_cast<uint32_t>( info_header.biHeight ),
        .mWidth     = static_cast<uint32_t>( info_header.biWidth ) };

    std::array blue_ns_data      = { engine::ImageBufferInitData{
        texture_buff_rgba.data(),
        static_cast<uint32_t>( texture_buff_rgba.size() ),
        static_cast<uint32_t>( info_header.biWidth * sizeof( uint32_t ) ) } };
    image_buffer_ci.mPreinitData = blue_ns_data;

    *res_buffer = device.CreateImageBuffer( image_buffer_ci );
}
