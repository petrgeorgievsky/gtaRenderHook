#include "bilateralblurpass.h"
#include <Engine/Common/IGPUAllocator.h>
#include <Engine/Common/IRenderingContext.h>
#include <Engine/D3D11Impl/ImageBuffers/D3D11Texture2D.h>
#include <Engine/D3D11Impl/ImageBuffers/ImageBuffer.h>
#include <Engine/IRenderer.h>

BilateralBlurPass::BilateralBlurPass()
{
    auto allocator = RHEngine::g_pRHRenderer->GetGPUAllocator();
    allocator->AllocateShader( {"shaders/d3d11/bilateral_blur.hlsl",
                                "Blur",
                                RHEngine::ShaderStage::Compute},
                               mBlurCS );

    RHEngine::ImageBufferInfo createInfo_{};
    createInfo_.type = RHEngine::ImageBufferType::RenderTargetBuffer;
    createInfo_.width = 1600;
    createInfo_.height = 900;
    createInfo_.mipLevels = 1;
    createInfo_.format = RHEngine::ImageBufferFormat::RGBA16;

    allocator->AllocateImageBuffer( createInfo_, mBlurV );
    allocator->AllocateImageBuffer( createInfo_, mResult );
}

void *BilateralBlurPass::Blur( void *image, void *gbNormals, void *gbPos )
{
    RHEngine::IRenderingContext *context = static_cast<RHEngine::IRenderingContext *>(
        RHEngine::g_pRHRenderer->GetCurrentContext() );
    context->BindShader( mBlurCS );
    context->BindImageBuffers( RHEngine::ImageBindType::UnorderedAccessTarget, {{0, mBlurV}} );
    context->BindImageBuffers( RHEngine::ImageBindType::CSResource,
                               {{0, image}, {1, gbNormals}, {2, gbPos}} );
    context->FlushCache();
    context->DispatchThreads( 1600 / 8, 900 / 8, 1 );
    context->ResetShader( mBlurCS );

    context->BindImageBuffers( RHEngine::ImageBindType::UnorderedAccessTarget, {{0, nullptr}} );
    context->BindImageBuffers( RHEngine::ImageBindType::CSResource,
                               {{0, nullptr}, {1, nullptr}, {2, nullptr}} );
    context->FlushCache();
    return mBlurV;
}
