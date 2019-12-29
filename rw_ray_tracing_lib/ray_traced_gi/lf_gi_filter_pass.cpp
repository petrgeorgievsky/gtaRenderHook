#include "lf_gi_filter_pass.h"
#include <Engine/Common/IRenderingContext.h>
#include <Engine/Common/types/constant_buffer_info.h>
#include <Engine/Common/types/image_bind_type.h>
#include <Engine/Common/types/image_buffer_format.h>
#include <Engine/Common/types/image_buffer_info.h>
#include <Engine/Common/types/image_buffer_type.h>
#include <Engine/Common/types/shader_info.h>
#include <Engine/Common/types/shader_stage.h>
#include <Engine/IRenderer.h>
#include <rw_engine/rw_macro_constexpr.h>
using namespace rh::engine;

rw_raytracing_lib::LowFreqGIFilterPass::LowFreqGIFilterPass( size_t w, size_t h, float lf_scale )
{
    auto allocator = g_pRHRenderer->GetGPUAllocator();
    allocator->AllocateShader( {"shaders/d3d11/lf_gi_filter.hlsl",
                                "LowFreqGIFilter",
                                ShaderStage::Compute},
                               mLowFreqGIFilterCS );
    allocator->AllocateShader( {"shaders/d3d11/lf_gi_prefilter.hlsl",
                                "LowFreqGIPreFilter",
                                ShaderStage::Compute},
                               mLowFreqGIPreFilterCS );
    dispatchThreadsX = w / mThreadGroupSize;
    dispatchThreadsY = h / mThreadGroupSize;
    dispatchThreadsZ = 1;
    ImageBufferInfo createInfo_{};
    createInfo_.type = ImageBufferType::RenderTargetBuffer;
    createInfo_.width = w;
    createInfo_.height = h;
    createInfo_.mipLevels = 1;
    createInfo_.format = ImageBufferFormat::RGBA8;
    allocator->AllocateImageBuffer( createInfo_, mFilterResult );

    createInfo_.type = ImageBufferType::RenderTargetBuffer;
    createInfo_.width = static_cast<uint32_t>( w * lf_scale );
    createInfo_.height = static_cast<uint32_t>( h * lf_scale );
    createInfo_.mipLevels = 1;
    createInfo_.format = ImageBufferFormat::RGBA16;
    allocator->AllocateImageBuffer( createInfo_, mPreFilteredSHY_Ping );
    allocator->AllocateImageBuffer( createInfo_, mPreFilteredSHY_Pong );

    createInfo_.type = ImageBufferType::RenderTargetBuffer;
    createInfo_.width = static_cast<uint32_t>( w * lf_scale );
    createInfo_.height = static_cast<uint32_t>( h * lf_scale );
    createInfo_.mipLevels = 1;
    createInfo_.format = ImageBufferFormat::RG16;
    allocator->AllocateImageBuffer( createInfo_, mPreFilteredSHCoCg_Ping );
    allocator->AllocateImageBuffer( createInfo_, mPreFilteredSHCoCg_Pong );
    ConstantBufferInfo cb_info{};
    cb_info.size = sizeof( LFFilterParams );
    allocator->AllocateConstantBuffer( cb_info, mPreFilterParamsCB );
    mPreFilterParams.fRenderingScale = lf_scale;
    IRenderingContext *context = static_cast<IRenderingContext *>(
        g_pRHRenderer->GetCurrentContext() );
    context->UpdateBuffer( mPreFilterParamsCB, reinterpret_cast<void *>( &mPreFilterParams ), -1 );
}

rw_raytracing_lib::LowFreqGIFilterPass::~LowFreqGIFilterPass()
{
    auto allocator = g_pRHRenderer->GetGPUAllocator();
    allocator->FreeImageBuffer( mFilterResult, ImageBufferType::RenderTargetBuffer );
}

void *rw_raytracing_lib::LowFreqGIFilterPass::Execute(
    void *gbPos, void *gbNormals, void *shY, void *shCoCg, void *gbColor, void *gbRadiance )
{
    IRenderingContext *context = static_cast<IRenderingContext *>(
        g_pRHRenderer->GetCurrentContext() );

    // Pre filter low frequency spherical harmonics
    mPreFilterParams.nPreFilterIteration = 0;
    PreFilter( shY, shCoCg, mPreFilteredSHY_Ping, mPreFilteredSHCoCg_Ping, gbPos, gbNormals );
    mPreFilterParams.nPreFilterIteration++;

    for ( int i = 0; i < 1; i++ ) {
        PreFilter( mPreFilteredSHY_Ping,
                   mPreFilteredSHCoCg_Ping,
                   mPreFilteredSHY_Pong,
                   mPreFilteredSHCoCg_Pong,
                   gbPos,
                   gbNormals );
        mPreFilterParams.nPreFilterIteration++;
        PreFilter( mPreFilteredSHY_Pong,
                   mPreFilteredSHCoCg_Pong,
                   mPreFilteredSHY_Ping,
                   mPreFilteredSHCoCg_Ping,
                   gbPos,
                   gbNormals );
        mPreFilterParams.nPreFilterIteration++;
    }

    PreFilter( mPreFilteredSHY_Ping,
               mPreFilteredSHCoCg_Ping,
               mPreFilteredSHY_Pong,
               mPreFilteredSHCoCg_Pong,
               gbPos,
               gbNormals );
    // Project LowFrequency SH
    context->BindShader( mLowFreqGIFilterCS );

    context->BindImageBuffers( ImageBindType::UnorderedAccessTarget, {{0, mFilterResult}} );

    context->BindImageBuffers( ImageBindType::CSResource,
                               {{0, gbPos},
                                {1, gbNormals},
                                {2, mPreFilteredSHY_Pong},
                                {3, mPreFilteredSHCoCg_Pong},
                                {4, gbColor},
                                {5, gbRadiance}} );

    context->FlushCache();

    context->DispatchThreads( dispatchThreadsX, dispatchThreadsY, dispatchThreadsZ );

    context->ResetShader( mLowFreqGIFilterCS );

    context->BindImageBuffers( ImageBindType::UnorderedAccessTarget, {{0, nullptr}} );
    context->BindImageBuffers( ImageBindType::CSResource,
                               {{0, nullptr},
                                {1, nullptr},
                                {2, nullptr},
                                {3, nullptr},
                                {4, nullptr},
                                {5, nullptr}} );
    context->FlushCache();
    return mFilterResult;
}

void rw_raytracing_lib::LowFreqGIFilterPass::PreFilter(
    void *shY_in, void *shCoCg_in, void *shY_out, void *shCoCg_out, void *gbPos, void *gbNormals )
{
    IRenderingContext *context = static_cast<IRenderingContext *>(
        g_pRHRenderer->GetCurrentContext() );
    context->UpdateBuffer( mPreFilterParamsCB, reinterpret_cast<void *>( &mPreFilterParams ), -1 );
    context->BindConstantBuffers( ShaderStage::Compute, {{1, mPreFilterParamsCB}} );
    // Pre filter low frequency spherical harmonics
    context->BindShader( mLowFreqGIPreFilterCS );

    context->BindImageBuffers( ImageBindType::UnorderedAccessTarget,
                               {{0, shY_out}, {1, shCoCg_out}} );
    context->BindImageBuffers( ImageBindType::CSResource,
                               {{0, gbPos}, {1, gbNormals}, {2, shY_in}, {3, shCoCg_in}} );
    context->FlushCache();

    context->DispatchThreads( dispatchThreadsX, dispatchThreadsY, dispatchThreadsZ );

    context->ResetShader( mLowFreqGIPreFilterCS );
    context->BindImageBuffers( ImageBindType::UnorderedAccessTarget, {{0, nullptr}, {1, nullptr}} );
    context->BindImageBuffers( ImageBindType::CSResource,
                               {{0, nullptr}, {1, nullptr}, {2, nullptr}, {3, nullptr}} );

    context->FlushCache();
}
