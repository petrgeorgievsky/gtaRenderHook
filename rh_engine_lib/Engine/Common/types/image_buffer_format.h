#pragma once
#include <cstdint>
namespace rh::engine
{
enum class ImageBufferFormat : uint8_t
{
    Unknown,
    BC1,
    BC2,
    BC3,
    BC4,
    BC5,
    BC6H,
    BC7,
    RGBA32,
    RGB32,
    RGBA16,
    RGB10A2,
    RG11B10,
    RGBA8,
    RG32,
    RG16,
    RG8,
    R32G8,
    R32,
    R16,
    R8,
    B5G6R5,
    BGR5A1,
    BGRA8,
    BGR8,
    A8,
    BGRA4,
    R8Uint,
    D24S8,
};
}
