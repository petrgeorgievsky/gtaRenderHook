//
// Created by peter on 14.01.2021.
//

#include "test_dump_util.h"
#include <Windows.h>
#include <array>
#include <fstream>
namespace rh::engine::tests
{
void DumpToBinaryFile( const std::string &name, char *data, std::size_t size )
{
    std::ofstream res( name, std::ios::out | std::ios::binary );
    res.write( data, size );
}
void DumpToBMP( const std::string &name, char *data, std::size_t width,
                std::size_t height )
{
    std::ofstream res( name, std::ios::out | std::ios::binary );

    BITMAPFILEHEADER f_header{ .bfType    = 0x4D42,
                               .bfOffBits = sizeof( BITMAPFILEHEADER ) +
                                            sizeof( BITMAPINFOHEADER ) };
    BITMAPINFOHEADER info_header{ .biSize = sizeof( BITMAPINFOHEADER ) };

    info_header.biWidth       = width;
    info_header.biHeight      = height;
    info_header.biPlanes      = 1;
    info_header.biBitCount    = 32;
    info_header.biCompression = BI_RGB;
    info_header.biSizeImage   = width * height * 4;

    res.write( reinterpret_cast<const char *>( &f_header ),
               sizeof( BITMAPFILEHEADER ) );
    res.write( reinterpret_cast<const char *>( &info_header ),
               sizeof( BITMAPINFOHEADER ) );

    res.write( data, width * height * 4 );
}
} // namespace rh::engine::tests