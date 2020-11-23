#pragma once
#include <cstdint>

namespace rh::engine
{

enum class LoadOp : uint8_t
{
    Load,
    Clear,
    DontCare
};

} // namespace rh::engine
