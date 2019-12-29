#include "rt_shadows_pass.h"
#include <Engine/Common/IRenderingContext.h>
#include <Engine/Common/types/image_bind_type.h>
#include <Engine/Common/types/image_buffer_format.h>
#include <Engine/Common/types/image_buffer_info.h>
#include <Engine/Common/types/image_buffer_type.h>
#include <Engine/Common/types/shader_info.h>
#include <Engine/Common/types/shader_stage.h>
#include <Engine/IRenderer.h>
using namespace rw_raytracing_lib;
using namespace rh::engine;

RTShadowsPass::RTShadowsPass( uint32_t w, uint32_t h )
{
    auto allocator = g_pRHRenderer->GetGPUAllocator();
    mDispatchParams.dispatchThreadsX = w / mThreadGroupSize;
    mDispatchParams.dispatchThreadsY = h / mThreadGroupSize;
    mDispatchParams.dispatchThreadsZ = 1;

    allocator->AllocateShader( {"shaders/d3d11/raytraced_shadows.hlsl",
                                "TraceRays",
                                ShaderStage::Compute},
                               mDispatchParams.shader );
    ImageBufferInfo createInfo_{};
    createInfo_.type = ImageBufferType::RenderTargetBuffer;
    createInfo_.width = w;
    createInfo_.height = h;
    createInfo_.mipLevels = 1;
    createInfo_.format = ImageBufferFormat::RGBA8;
    allocator->AllocateImageBuffer( createInfo_, mDispatchParams.target );
}

RTShadowsPass::~RTShadowsPass()
{
    auto allocator = g_pRHRenderer->GetGPUAllocator();
    allocator->FreeImageBuffer( mDispatchParams.target, ImageBufferType::RenderTargetBuffer );
}

void *RTShadowsPass::Execute( RayTracer *ray_tracer, void *gbPos, void *gbNormals )
{
    IRenderingContext *context = static_cast<IRenderingContext *>(
        g_pRHRenderer->GetCurrentContext() );
    context->BindImageBuffers( ImageBindType::CSResource, {{4, gbPos}, {5, gbNormals}} );
    ray_tracer->TraceRays( mDispatchParams );
    context->BindImageBuffers( ImageBindType::CSResource, {{4, nullptr}, {5, nullptr}} );
    context->FlushCache();
    return mDispatchParams.target;
}
