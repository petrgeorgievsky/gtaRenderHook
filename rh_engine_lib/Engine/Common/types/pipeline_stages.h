#pragma once
#include <cstdint>

namespace rh::engine
{

enum class PipelineStage : uint32_t
{
    PrePass,
    DrawIndirect,
    VertexInput,
    VertexShader,
    TessControl,
    TessEvaluation,
    GeometryShader,
    PixelShader,
    EarlyFragmentTests,
    LateFragmentTests,
    ColorAttachmentOutput,
    ComputeShader,
    Transfer,
    PostPass,
    Host,
    GraphicsPipelineEnd,
    AllPipelineEnd,
    BuildAcceleration,
    /// Vulkan EXTENSIONS
    // AllPipelineEnd,
    RayTracing,
};

} // namespace rh::engine
