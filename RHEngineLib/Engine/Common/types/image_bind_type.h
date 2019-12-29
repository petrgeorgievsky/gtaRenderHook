#pragma once
namespace rh::engine {
/*
    This enum contains all possible image buffer to pipeline bindings
*/
enum class ImageBindType : unsigned char {
    Unknown,
    RenderTarget,
    DepthStencilTarget,
    UnorderedAccessTarget,
    PSResource,
    CSResource,
    VSResource,
    GSResource,
    DSResource,
    HSResource
};
} // namespace rh::engine
