#pragma once
#include <cstdint>

namespace rh::engine
{

enum class SamplerFilter : uint8_t
{
    Unknown = 0,
    Point,
    Linear,
    Anisotropic
};

}
