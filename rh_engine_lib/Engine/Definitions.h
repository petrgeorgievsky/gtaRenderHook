#pragma once

#include <cstdint>
namespace rh::engine
{

enum class RenderingAPI : uint32_t
{
    DX11,
    Vulkan
};
constexpr bool gDebugEnabled =
#ifdef _DEBUG
    true
#else
    false
#endif
    ;
}; // namespace rh::engine
