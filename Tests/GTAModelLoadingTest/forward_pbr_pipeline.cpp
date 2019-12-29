#include "forward_pbr_pipeline.h"
#include <Engine/Common/IRenderingContext.h>
#include <Engine/Common/types/constant_buffer_info.h>
#include <Engine/Common/types/input_element_type.h>
#include <Engine/Common/types/input_layout_info.h>
#include <Engine/Common/types/shader_info.h>
#include <Engine/Common/types/shader_stage.h>
#include <Engine/D3D11Impl/Buffers/D3D11ConstantBuffer.h>
#include <Engine/D3D11Impl/D3D11PrimitiveBatch.h>
#include <Engine/IRenderer.h>
#include <RWUtils/RwAPI.h>
#include <rw_engine/rw_rh_pipeline.h>
#include <rw_engine/rw_texture/rw_texture.h>

ForwardPBRPipeline::ForwardPBRPipeline()
{
    rh::engine::IGPUAllocator *allocator = rh::engine::g_pRHRenderer->GetGPUAllocator();

    allocator->AllocateShader( {TEXT( "shaders/d3d11/forward_pbr_pipeline.hlsl" ),
                                TEXT( "BaseVS" ),
                                rh::engine::ShaderStage::Vertex},
                               mBaseVS );
    allocator->AllocateShader( {TEXT( "shaders/d3d11/forward_pbr_pipeline.hlsl" ),
                                TEXT( "NoTexPS" ),
                                rh::engine::ShaderStage::Pixel},
                               mNoTexPS );
    allocator->AllocateShader( {TEXT( "shaders/d3d11/forward_pbr_pipeline.hlsl" ),
                                TEXT( "TexPS" ),
                                rh::engine::ShaderStage::Pixel},
                               mTexPS );
    allocator->AllocateShader( {TEXT( "shaders/d3d11/forward_pbr_pipeline.hlsl" ),
                                TEXT( "SelectedPS" ),
                                rh::engine::ShaderStage::Pixel},
                               mSelectedPS );

    rh::engine::InputLayoutInfo layout_info;
    layout_info.inputElements = {{"POSITION", rh::engine::InputElementType::Vec4fp32},
                                 {"COLOR", rh::engine::InputElementType::Vec4fp8},
                                 {"TEXCOORD", rh::engine::InputElementType::Vec2fp32},
                                 {"NORMAL", rh::engine::InputElementType::Vec3fp32}};
    layout_info.shaderPtr = mBaseVS;
    allocator->AllocateInputLayout( layout_info, mVertexDecl );
    allocator->AllocateConstantBuffer( {sizeof( MaterialCB )}, mMaterialCB );
}

ForwardPBRPipeline::~ForwardPBRPipeline()
{
    delete static_cast<rh::engine::D3D11ConstantBuffer *>( mMaterialCB );
}

void ForwardPBRPipeline::DrawMesh( rh::engine::IRenderingContext *context,
                                   rh::engine::IPrimitiveBatch *model )
{
    uint32_t strides[] = {sizeof( rh::rw::engine::VertexDescPosColorUVNormals )};
    uint32_t offsets[] = {0};
    context->BindShader( mBaseVS );

    context->BindInputLayout( mVertexDecl );
    context->SetPrimitiveTopology( model->PrimType() );
    context->BindIndexBuffer( model->IndexBuffer() );
    context->BindVertexBuffers( {model->VertexBuffer()}, strides, offsets );

    for ( const rh::engine::MeshSplitData &meshData : model->SplitData() ) {
        RpMaterial *material = static_cast<RpMaterial *>( meshData.material );

        if ( material && material->texture ) {
            rh::rw::engine::RwD3D11SetTexture(
                context,
                material->texture,
                0,
                static_cast<uint32_t>( rh::rw::engine::RwRenderShaderStage::PixelShader ) );

            context->UpdateBuffer( mMaterialCB, &mDefMaterialCBData, -1 );
            context->BindConstantBuffers( rh::engine::ShaderStage::Pixel, {{2, mMaterialCB}} );
            context->BindShader( mTexPS );
        } else {
            rh::rw::engine::RwD3D11SetTexture(
                context,
                nullptr,
                0,
                static_cast<uint32_t>( rh::rw::engine::RwRenderShaderStage::PixelShader ) );

            context->BindShader( mNoTexPS );
        }

        context->FlushCache();
        context->RecordDrawCallIndexed( meshData.numIndex, meshData.startIndex, 0 );
    }
}
