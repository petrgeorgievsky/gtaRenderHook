//
// Created by peter on 14.01.2021.
//
#pragma once
#include <string>
namespace rh::engine::tests
{
/// Dumps data to .bin file
void DumpToBinaryFile( const std::string &name, char *data, std::size_t size );
/// Dumps RGB data to .bmp file
void DumpToBMP( const std::string &name, char *data, std::size_t width,
                std::size_t height );
} // namespace rh::engine::tests
