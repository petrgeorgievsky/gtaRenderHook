#include "forward_pbr_pipeline.h"
#include <Engine/Common/IRenderingContext.h>
#include <Engine/D3D11Impl/Buffers/D3D11ConstantBuffer.h>
#include <Engine/D3D11Impl/D3D11PrimitiveBatch.h>
#include <Engine/Definitions.h>
#include <Engine/IRenderer.h>
#include <rw_engine/rw_rh_pipeline.h>
#include <rw_engine/rw_texture/rw_texture.h>

ForwardPBRPipeline::ForwardPBRPipeline()
{
    RHEngine::IGPUAllocator *allocator = RHEngine::g_pRHRenderer->GetGPUAllocator();

    allocator->AllocateShader( {TEXT( "shaders/d3d11/forward_pbr_pipeline.hlsl" ),
                                TEXT( "BaseVS" ),
                                RHEngine::RHShaderStage::Vertex},
                               mBaseVS );
    allocator->AllocateShader( {TEXT( "shaders/d3d11/forward_pbr_pipeline.hlsl" ),
                                TEXT( "NoTexPS" ),
                                RHEngine::RHShaderStage::Pixel},
                               mNoTexPS );
    allocator->AllocateShader( {TEXT( "shaders/d3d11/forward_pbr_pipeline.hlsl" ),
                                TEXT( "TexPS" ),
                                RHEngine::RHShaderStage::Pixel},
                               mTexPS );
    allocator->AllocateShader( {TEXT( "shaders/d3d11/forward_pbr_pipeline.hlsl" ),
                                TEXT( "SelectedPS" ),
                                RHEngine::RHShaderStage::Pixel},
                               mSelectedPS );

    RHEngine::RHInputLayoutInfo layout_info;
    layout_info.inputElements = {{RHEngine::RHInputElementType::Vec4fp32, "POSITION"},
                                 {RHEngine::RHInputElementType::Vec4fp8, "COLOR"},
                                 {RHEngine::RHInputElementType::Vec2fp32, "TEXCOORD"},
                                 {RHEngine::RHInputElementType::Vec3fp32, "NORMAL"}};
    layout_info.shaderPtr = mBaseVS;
    allocator->AllocateInputLayout( layout_info, mVertexDecl );
    allocator->AllocateConstantBuffer( {sizeof( MaterialCB )}, mMaterialCB );
}

ForwardPBRPipeline::~ForwardPBRPipeline()
{
    delete static_cast<RHEngine::D3D11ConstantBuffer *>( mMaterialCB );
}

void ForwardPBRPipeline::DrawMesh( RHEngine::IRenderingContext *context,
                                   RHEngine::IPrimitiveBatch *model )
{
    uint32_t strides[] = {sizeof( rw_rh_engine::VertexDescPosColorUVNormals )};
    uint32_t offsets[] = {0};
    context->BindShader( mBaseVS );

    context->BindInputLayout( mVertexDecl );
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

            context->UpdateBuffer( mMaterialCB, &mDefMaterialCBData, -1 );
            context->BindConstantBuffers( RHEngine::RHShaderStage::Pixel, {{2, mMaterialCB}} );
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
