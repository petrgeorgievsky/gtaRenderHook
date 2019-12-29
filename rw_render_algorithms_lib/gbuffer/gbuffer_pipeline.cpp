#include "gbuffer_pipeline.h"
#include <Engine/Common/IRenderingContext.h>
#include <Engine/Common/types/input_element_type.h>
#include <Engine/Common/types/input_layout_info.h>
#include <Engine/Common/types/shader_info.h>
#include <Engine/Common/types/shader_stage.h>
#include <Engine/IRenderer.h>
#include <game_sa/RenderWare.h>
#include <rw_engine/rw_rh_pipeline.h>
#include <rw_engine/rw_texture/rw_texture.h>

GBufferPipeline::GBufferPipeline()
{
    rh::engine::IGPUAllocator *allocator = rh::engine::g_pRHRenderer->GetGPUAllocator();

    rh::engine::IGPUResource *resource_ptr = nullptr;
    allocator->AllocateShader( {TEXT( "shaders/d3d11/gbuffer_pipeline.hlsl" ),
                                TEXT( "BaseVS" ),
                                rh::engine::ShaderStage::Vertex},
                               resource_ptr );
    mBaseVS = resource_ptr;
    allocator->AllocateShader( {TEXT( "shaders/d3d11/gbuffer_pipeline.hlsl" ),
                                TEXT( "NoTexPS" ),
                                rh::engine::ShaderStage::Pixel},
                               resource_ptr );
    mNoTexPS = resource_ptr;
    allocator->AllocateShader( {TEXT( "shaders/d3d11/gbuffer_pipeline.hlsl" ),
                                TEXT( "TexPS" ),
                                rh::engine::ShaderStage::Pixel},
                               resource_ptr );
    mTexPS = resource_ptr;

    rh::engine::InputLayoutInfo layout_info;
    layout_info.inputElements = {{"POSITION", rh::engine::InputElementType::Vec4fp32},
                                 {"COLOR", rh::engine::InputElementType::Vec4fp8},
                                 {"TEXCOORD", rh::engine::InputElementType::Vec2fp32},
                                 {"NORMAL", rh::engine::InputElementType::Vec3fp32}};
    layout_info.shaderPtr = mBaseVS.mPtr;
    allocator->AllocateInputLayout( layout_info, resource_ptr );
    mVertexDecl = resource_ptr;
}

GBufferPipeline::~GBufferPipeline() {}

void GBufferPipeline::DrawMesh( rh::engine::IRenderingContext *context,
                                rh::engine::IPrimitiveBatch *model )
{
    uint32_t strides[] = {sizeof( rh::rw::engine::VertexDescPosColorUVNormals )};
    uint32_t offsets[] = {0};
    context->BindShader( mBaseVS.mPtr );

    context->BindInputLayout( mVertexDecl.mPtr );
    context->SetPrimitiveTopology( model->PrimType() );
    context->BindIndexBuffer( model->IndexBuffer() );
    context->BindVertexBuffers( {model->VertexBuffer()}, strides, offsets );

    for ( const rh::engine::MeshSplitData &meshData : model->SplitData() ) {
        RpMaterial *material = static_cast<RpMaterial *>( meshData.material );

        if ( material && material->texture ) {
            rh::rw::engine::RwD3D11SetTexture( context,
                                             material->texture,
                                             0,
                                             static_cast<uint32_t>(
                                                 rh::rw::engine::RwRenderShaderStage::PixelShader ) );

            context->BindShader( mTexPS.mPtr );
        } else {
            rh::rw::engine::RwD3D11SetTexture( context,
                                             nullptr,
                                             0,
                                             static_cast<uint32_t>(
                                                 rh::rw::engine::RwRenderShaderStage::PixelShader ) );

            context->BindShader( mNoTexPS.mPtr );
        }

        context->FlushCache();
        context->RecordDrawCallIndexed( meshData.numIndex, meshData.startIndex, 0 );
    }
}
