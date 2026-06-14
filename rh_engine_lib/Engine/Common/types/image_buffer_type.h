#pragma once
#include <cstdint>

namespace rh::engine {

enum class ImageBufferType : uint8_t {
    Unknown,
    BackBuffer,
    TextureBuffer,
    DepthBuffer,
    RenderTargetBuffer,
    DynamicTextureArrayBuffer
};

}
