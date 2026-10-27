#pragma once
#include <cstdint>

namespace rh::rw::engine::restir
{

struct DispatchSize
{
    uint32_t x;
    uint32_t y;
    uint32_t z;
};

// Number of compute groups needed to cover a width x height image.
// Rounds up: a partial edge tile still has to be dispatched, and the compute
// shaders early-out for invocations outside the image.
inline DispatchSize ComputeDispatchSize( uint32_t width, uint32_t height,
                                         uint32_t local_size )
{
    return { ( width + local_size - 1 ) / local_size,
             ( height + local_size - 1 ) / local_size, 1 };
}

} // namespace rh::rw::engine::restir
