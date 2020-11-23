#pragma once
namespace rh::engine {
enum class ImageBufferType : unsigned char {
    Unknown,
    BackBuffer,
    TextureBuffer,
    DepthBuffer,
    RenderTargetBuffer,
    DynamicTextureArrayBuffer
};
}
