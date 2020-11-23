#pragma once
#include <cstdint>
namespace rh::engine
{
enum ShaderStage : uint32_t
{
    Compute   = 1,
    Domain    = 2,
    Geometry  = 4,
    Hull      = 8,
    Pixel     = 16,
    Vertex    = 32,
    RayGen    = 64,
    RayMiss   = 128,
    RayHit    = 256,
    RayAnyHit = 512,
};
}
