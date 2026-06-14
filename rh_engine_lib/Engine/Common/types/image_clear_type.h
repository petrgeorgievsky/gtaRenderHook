#pragma once
#include <cstdint>

namespace rh::engine {

/*
    This enum contains all possible image buffer clear types
*/
enum class ImageClearType : uint8_t { Color, Depth, Stencil, DepthStencil };
} // namespace rh::engine
