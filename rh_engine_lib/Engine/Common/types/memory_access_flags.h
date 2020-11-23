#pragma once
#include <cstdint>

namespace rh::engine
{

enum class MemoryAccessFlags : uint32_t
{
    Unknown,
    IndirectCommandRead,
    IndexRead,
    VertexAttributeRead,
    UniformRead,
    InputAttachmentRead,
    ShaderRead,
    ShaderWrite,
    ColorAttachmentRead,
    ColorAttachmentWrite,
    DepthStencilAttachmentRead,
    DepthStencilAttachmentWrite,
    TransferRead,
    TransferWrite,
    HostRead,
    HostWrite,
    MemoryRead,
    MemoryWrite,
    /// Vulkan EXTENSIONS
    // AllPipelineEnd,

    AccelerationStructureWrite,
    AccelerationStructureRead,
};

} // namespace rh::engine
