#pragma once
#include <cstdint>

namespace rh::engine
{

enum class PipelineBindPoint : uint8_t
{
    Graphics,
    Compute,
    RayTracing
};

} // namespace rh::engine
