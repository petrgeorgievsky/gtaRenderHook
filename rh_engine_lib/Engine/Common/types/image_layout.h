#pragma once
#include <cstdint>

namespace rh::engine
{
enum class ImageLayout : uint8_t
{
    Undefined,
    General,
    ColorAttachment,
    DepthStencilAttachment,
    DepthStencilReadOnly,
    ShaderReadOnly,
    TransferSrc,
    TransferDst,
    Preinitialized,
    DepthReadOnlyStencil,
    DepthAttachmentStencilReadOnly,
    PresentSrc,
    SharedPresent,
    ShadingRateOptimal,
    FragmentDensityMapOptimal
};
}