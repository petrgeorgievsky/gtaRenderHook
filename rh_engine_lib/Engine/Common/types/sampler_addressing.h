#pragma once
#include <cstdint>

namespace rh::engine
{

enum class SamplerAddressing : uint8_t
{
    Unknown = 0,
    Wrap,
    Mirror,
    Clamp,
    Border
};

}
