#include "forward_pbr_pipeline.h"
#include <Engine/Common/IRenderingContext.h>
#include <Engine/D3D11Impl/D3D11PrimitiveBatch.h>
#include <RWUtils/RwAPI.h>
#include <Engine/Definitions.h>
#include <Engine/IRenderer.h>
#include <Engine/D3D11Impl/Buffers/D3D11ConstantBuffer.h>

ForwardPBRPipeline::ForwardPBRPipeline( IMaterialBufferProvider* matbuf_prov ): 
    mMatBufferProvider( matbuf_prov )
{
    RHEngine::IGPUAllocator* allocator = RHEngine::g_pRHRenderer->GetGPUAllocator();

    allocator->AllocateShader( { TEXT( "shaders/d3d11/forward_pbr_pipeline.hlsl" ), TEXT( "BaseVS" ), RHEngine::RHShaderStage::Vertex },
                               mBaseVS );
    allocator->AllocateShader( { TEXT( "shaders/d3d11/forward_pbr_pipeline.hlsl" ), TEXT( "NoTexPS" ), RHEngine::RHShaderStage::Pixel },
                               mNoTexPS );
    allocator->AllocateShader( { TEXT( "shaders/d3d11/forward_pbr_pipeline.hlsl" ), TEXT( "TexPS" ), RHEngine::RHShaderStage::Pixel },
                               mTexPS );
    allocator->AllocateShader( { TEXT( "shaders/d3d11/forward_pbr_pipeline.hlsl" ), TEXT( "SelectedPS" ), RHEngine::RHShaderStage::Pixel },
                               mSelectedPS );
    RHEngine::RHInputLayoutInfo layout_info;
    layout_info.inputElements =
    {
        { RHEngine::RHInputElementType::Vec4fp32, "POSITION" },
        { RHEngine::RHInputElementType::Vec4fp8, "COLOR" },
        { RHEngine::RHInputElementType::Vec2fp32, "TEXCOORD" },
        { RHEngine::RHInputElementType::Vec3fp32, "NORMAL" }
    };
    layout_info.shaderPtr = mBaseVS;
    allocator->AllocateInputLayout( layout_info, mVertexDecl );
    allocator->AllocateConstantBuffer( { sizeof( MaterialCB ) }, (void*&)mMaterialCB );
}

ForwardPBRPipeline::~ForwardPBRPipeline()
{
    delete mMaterialCB;
}

void ForwardPBRPipeline::DrawObject( RHEngine::IRenderingContext* context, RHEngine::D3D11PrimitiveBatch* model,
                                     const PBRRenderingParams& params )
{
    uint32_t strides[] = { sizeof( RHVertexDescPosColorUVNormals ) };
    uint32_t offsets[] = { 0 };
    context->BindShader( mBaseVS );

    context->BindInputLayout( mVertexDecl );
    context->SetPrimitiveTopology( model->PrimitiveType() );
    context->BindIndexBuffer( model->IndexBuffer() );
    context->BindVertexBuffers( { model->VertexBuffer() }, strides, offsets );

    if( params.highlightModel )
        context->BindShader( mSelectedPS );
    for( const RHEngine::MeshSplitData& meshData : model->SplitData() )
    {
        if( !params.highlightModel )
        {
            if( meshData.material&& meshData.material->texture )
            {
                RH_RWAPI::RwD3D11SetTexture( context, meshData.material->texture, 0, RHEngine::RwRenderShaderStage::PixelShader );
                if( mMatBufferProvider && 
                    mMatBufferProvider->GetBuffer( std::string( meshData.material->texture->name ) ) != nullptr )
                {
                    context->UpdateBuffer( (void*)mMaterialCB, (void*)mMatBufferProvider->GetBuffer( std::string( meshData.material->texture->name ) ), -1 );
                }
                else 
                {
                    context->UpdateBuffer( (void*)mMaterialCB, (void*)& mDefMaterialCBData, -1 );
                }
                context->BindConstantBuffers( RHEngine::RHShaderStage::Pixel, { { 2, (void*)mMaterialCB } } );
                context->BindShader( mTexPS );
            }
            else
            {
                RH_RWAPI::RwD3D11SetTexture( context, nullptr, 0, RHEngine::RwRenderShaderStage::PixelShader );

                context->BindShader( mNoTexPS );
            }
        }
        else {
            RH_RWAPI::RwD3D11SetTexture( context, nullptr, 0, RHEngine::RwRenderShaderStage::PixelShader );
        }
        context->FlushCache();
        context->RecordDrawCallIndexed( meshData.numIndex, meshData.startIndex, 0 );
    }
}
