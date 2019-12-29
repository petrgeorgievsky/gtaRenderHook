#include "irradiance_fields_gi_pass.h"
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

RTIrradianceGIPass::RTIrradianceGIPass( uint32_t sx, uint32_t sy, uint32_t sz )
{
    auto allocator = g_pRHRenderer->GetGPUAllocator();
    mDispatchParams.dispatchThreadsX = sx;
    mDispatchParams.dispatchThreadsY = sy;
    mDispatchParams.dispatchThreadsZ = sz;
    allocator->AllocateShader( {"shaders/d3d11/irradiance_field_prepass.hlsl",
                                "TraceRays",
                                ShaderStage::Compute},
                               mDispatchParams.shader );
    allocator->AllocateShader( {"shaders/d3d11/irradiance_field_composite.hlsl",
                                "Composite",
                                ShaderStage::Compute},
                               mIrradianceFieldCS );
    ImageBufferInfo createInfo_{};
    createInfo_.type = ImageBufferType::RenderTargetBuffer;
    createInfo_.width = sx * 4;
    createInfo_.height = sy * sz * 4;
    createInfo_.mipLevels = 1;
    createInfo_.format = ImageBufferFormat::RGBA8;
    allocator->AllocateImageBuffer( createInfo_, mDispatchParams.target );
}

RTIrradianceGIPass::~RTIrradianceGIPass()
{
    auto allocator = g_pRHRenderer->GetGPUAllocator();
    allocator->FreeImageBuffer( mDispatchParams.target, ImageBufferType::RenderTargetBuffer );
}

void RTIrradianceGIPass::ComputeIrradianceField( RayTracer *ray_tracer )
{
    IRenderingContext *context = static_cast<IRenderingContext *>(
        g_pRHRenderer->GetCurrentContext() );
    ray_tracer->TraceRays( mDispatchParams );
    context->FlushCache();
}

void RTIrradianceGIPass::Execute( void *gbPos, void *gbNormals, void *result, size_t w, size_t h )
{
    IRenderingContext *context = static_cast<IRenderingContext *>(
        g_pRHRenderer->GetCurrentContext() );
    context->BindShader( mIrradianceFieldCS );

    context->BindImageBuffers( ImageBindType::UnorderedAccessTarget, {{0, result}} );

    context->BindImageBuffers( ImageBindType::CSResource,
                               {{0, gbPos}, {1, gbNormals}, {2, mDispatchParams.target}} );

    context->FlushCache();

    context->DispatchThreads( w / mThreadGroupSizeComposite, h / mThreadGroupSizeComposite, 1 );

    context->ResetShader( mIrradianceFieldCS );

    context->BindImageBuffers( ImageBindType::UnorderedAccessTarget, {{0, nullptr}} );
    context->BindImageBuffers( ImageBindType::CSResource, {{0, nullptr}, {1, nullptr}} );
    context->FlushCache();
}
