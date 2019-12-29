#include "gbuffer_pipeline.h"
#include <Engine/D3D11Impl/ImageBuffers/D3D11Texture2D.h>
#include <Engine/IRenderer.h>
#include <rw_engine/rw_rh_pipeline.h>
#include <rw_engine/rw_texture/rw_texture.h>

GBufferPipeline::GBufferPipeline()
{
    RHEngine::IGPUAllocator *allocator = RHEngine::g_pRHRenderer->GetGPUAllocator();

    allocator->AllocateShader( {TEXT( "shaders/d3d11/gbuffer_pipeline.hlsl" ),
                                TEXT( "BaseVS" ),
                                RHEngine::ShaderStage::Vertex},
                               mBaseVS );
    allocator->AllocateShader( {TEXT( "shaders/d3d11/gbuffer_pipeline.hlsl" ),
                                TEXT( "NoTexPS" ),
                                RHEngine::ShaderStage::Pixel},
                               mNoTexPS );
    allocator->AllocateShader( {TEXT( "shaders/d3d11/gbuffer_pipeline.hlsl" ),
                                TEXT( "TexPS" ),
                                RHEngine::ShaderStage::Pixel},
                               mTexPS );

    RHEngine::RHInputLayoutInfo layout_info;
    layout_info.inputElements = {{RHEngine::RHInputElementType::Vec4fp32, "POSITION"},
                                 {RHEngine::RHInputElementType::Vec4fp8, "COLOR"},
                                 {RHEngine::RHInputElementType::Vec2fp32, "TEXCOORD"},
                                 {RHEngine::RHInputElementType::Vec3fp32, "NORMAL"}};
    layout_info.shaderPtr = mBaseVS;
    allocator->AllocateInputLayout( layout_info, mVertexDecl );

    RHEngine::ImageBufferInfo createInfo_{};
    createInfo_.type = RHEngine::ImageBufferType::RenderTargetBuffer;
    createInfo_.width = 1600;
    createInfo_.height = 900;
    createInfo_.mipLevels = 1;
    createInfo_.format = RHEngine::ImageBufferFormat::RGBA32;

    allocator->AllocateImageBuffer( createInfo_, mGBufferTexture[0] );
    createInfo_.format = RHEngine::ImageBufferFormat::RGBA16;
    allocator->AllocateImageBuffer( createInfo_, mGBufferTexture[1] );
    allocator->AllocateImageBuffer( createInfo_, mGBufferTexture[2] );
    allocator->AllocateImageBuffer( createInfo_, mGBufferTexture[3] );

    createInfo_.type = RHEngine::ImageBufferType::DepthBuffer;
    createInfo_.format = RHEngine::ImageBufferFormat::R32;

    allocator->AllocateImageBuffer( createInfo_, mGBufferDepthTexture );
}

GBufferPipeline::~GBufferPipeline() {}

void GBufferPipeline::DrawMesh( RHEngine::IRenderingContext *context,
                                RHEngine::IPrimitiveBatch *model )
{
    uint32_t strides[] = {sizeof( rw_rh_engine::VertexDescPosColorUVNormals )};
    uint32_t offsets[] = {0};

    context->SetPrimitiveTopology( model->PrimitiveType() );
    context->BindIndexBuffer( model->IndexBuffer() );
    context->BindVertexBuffers( {model->VertexBuffer()}, strides, offsets );

    for ( const RHEngine::MeshSplitData &meshData : model->SplitData() ) {
        RpMaterial *material = static_cast<RpMaterial *>( meshData.material );

        if ( material && material->texture ) {
            rw_rh_engine::RwD3D11SetTexture( context,
                                             material->texture,
                                             0,
                                             static_cast<uint32_t>(
                                                 rw_rh_engine::RwRenderShaderStage::PixelShader ) );

            context->BindShader( mTexPS );
        } else {
            rw_rh_engine::RwD3D11SetTexture( context,
                                             nullptr,
                                             0,
                                             static_cast<uint32_t>(
                                                 rw_rh_engine::RwRenderShaderStage::PixelShader ) );

            context->BindShader( mNoTexPS );
        }

        context->FlushCache();
        context->RecordDrawCallIndexed( meshData.numIndex, meshData.startIndex, 0 );
    }
}

void GBufferPipeline::PrepareFrame( RHEngine::IRenderingContext *context )
{
    float clear_color[4] = {0, 0, 0, 0};
    context->ClearImageBuffer( RHEngine::ImageClearType::Color, mGBufferTexture[0], clear_color );
    context->ClearImageBuffer( RHEngine::ImageClearType::Color, mGBufferTexture[1], clear_color );
    context->ClearImageBuffer( RHEngine::ImageClearType::Color, mGBufferTexture[2], clear_color );
    context->ClearImageBuffer( RHEngine::ImageClearType::Color, mGBufferTexture[3], clear_color );
    context->ClearImageBuffer( RHEngine::ImageClearType::Depth, mGBufferDepthTexture, clear_color );
    context->BindImageBuffers( RHEngine::ImageBindType::DepthStencilTarget,
                               {{0, mGBufferDepthTexture}} );

    context->BindImageBuffers( RHEngine::ImageBindType::RenderTarget,
                               {{0, mGBufferTexture[0]},
                                {1, mGBufferTexture[1]},
                                {2, mGBufferTexture[2]},
                                {3, mGBufferTexture[3]}} );

    context->BindShader( mBaseVS );

    context->BindInputLayout( mVertexDecl );
}

void GBufferPipeline::EndFrame( RHEngine::IRenderingContext *context )
{
    context->BindImageBuffers( RHEngine::ImageBindType::RenderTarget,
                               {{0, nullptr}, {1, nullptr}, {2, nullptr}, {3, nullptr}} );
    context->BindImageBuffers( RHEngine::ImageBindType::DepthStencilTarget, {{0, nullptr}} );
}

void *GBufferPipeline::GetGBuffer( uint32_t id )
{
    return mGBufferTexture[id];
}
