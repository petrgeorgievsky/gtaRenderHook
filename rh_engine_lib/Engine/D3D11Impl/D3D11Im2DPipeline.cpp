#include "D3D11Im2DPipeline.h"
#include "D3D11Common.h"
#include "Engine/Common/IRenderingContext.h"
#include "Engine/Common/types/comparison_func.h"
#include "Engine/Common/types/image_bind_type.h"
#include "Engine/Common/types/index_buffer_info.h"
#include "Engine/Common/types/index_ptr_pair.h"
#include "Engine/Common/types/input_element_type.h"
#include "Engine/Common/types/input_layout_info.h"
#include "Engine/Common/types/primitive_type.h"
#include "Engine/Common/types/sampler.h"
#include "Engine/Common/types/sampler_addressing.h"
#include "Engine/Common/types/sampler_filter.h"
#include "Engine/Common/types/shader_info.h"
#include "Engine/Common/types/shader_stage.h"
#include "Engine/Common/types/vertex_buffer_info.h"
#include <d3d11.h>

const unsigned int rh::engine::D3D11Im2DPipeline::m_nVertexBufferCapacity = 240000;
const unsigned int rh::engine::D3D11Im2DPipeline::m_nIndexBufferCapacity = 240000;

rh::engine::D3D11Im2DPipeline::D3D11Im2DPipeline( IGPUAllocator &allocator )
{
    IGPUResource *resource_ptr;
    ShaderInfo shader_info = {TEXT( "shaders/d3d11/engine/Im2D.hlsl" ),
                              TEXT( "BaseVS" ),
                              ShaderStage::Vertex};

    allocator.AllocateShader( shader_info, resource_ptr );
    m_pBaseVS = resource_ptr;

    shader_info = {TEXT( "shaders/d3d11/engine/Im2D.hlsl" ), TEXT( "NoTexPS" ), ShaderStage::Pixel};
    allocator.AllocateShader( shader_info, resource_ptr );
    m_pNoTexPS = resource_ptr;

    shader_info = {TEXT( "shaders/d3d11/engine/Im2D.hlsl" ), TEXT( "TexPS" ), ShaderStage::Pixel};
    allocator.AllocateShader( shader_info, resource_ptr );
    m_pTexPS = resource_ptr;

    InputLayoutInfo layout_info;
    layout_info.inputElements = {{"POSITION", InputElementType::Vec4fp32},
                                 {"COLOR", InputElementType::Vec4fp8},
                                 {"TEXCOORD", InputElementType::Vec2fp32}};
    layout_info.shaderPtr = m_pBaseVS.mPtr;
    allocator.AllocateInputLayout( layout_info, resource_ptr );
    m_pVertexDecl = resource_ptr;

    allocator
        .AllocateVertexBuffer( {true, sizeof( Simple2DVertex ), m_nVertexBufferCapacity, nullptr},
                               resource_ptr );
    m_pVertexBuffer = resource_ptr;

    allocator.AllocateIndexBuffer( {true, m_nIndexBufferCapacity, nullptr}, resource_ptr );
    m_pIndexBuffer = resource_ptr;

    //m_pVertexBuffer->SetDebugName( TEXT( "Im2D_DynamicVB" ) );
    //m_pIndexBuffer->SetDebugName( TEXT( "Im2D_DynamicIB" ) );
}

rh::engine::D3D11Im2DPipeline::~D3D11Im2DPipeline()
{
    rh::debug::DebugLogger::Log( "D3D11Im2DPipeline destructor..." );
}

void rh::engine::D3D11Im2DPipeline::Shutdown() {}

void rh::engine::D3D11Im2DPipeline::Draw( void *impl,
                                        PrimitiveType prim,
                                        Simple2DVertex *verticles,
                                        unsigned int vertexCount )
{
    auto *context = static_cast<IRenderingContext *>( impl );

    RH_ASSERT( vertexCount > 0 )

    // dynamic vertex buffer update
    if ( prim != PrimitiveType::TriangleFan )
        context->UpdateBuffer( m_pVertexBuffer.mPtr,
                               verticles,
                               static_cast<int32_t>( sizeof( Simple2DVertex ) * vertexCount ) );
    else {
        std::vector<Simple2DVertex> vertexArr;
        vertexArr.reserve( ( vertexCount - 2 ) * 3 );

        // d3d11 doesn't have triangle fan so we need to convert it to triangle list
        for ( unsigned int i = 1; i < vertexCount - 1; i++ ) {
            vertexArr.emplace_back( verticles[0] );
            vertexArr.emplace_back( verticles[i] );
            vertexArr.emplace_back( verticles[i + 1] );
        }

        vertexCount = static_cast<UINT>( vertexArr.size() );
        context->UpdateBuffer( m_pVertexBuffer.mPtr,
                               vertexArr.data(),
                               sizeof( Simple2DVertex ) * vertexCount );
    }

    // initialize primitive info
    context->BindInputLayout( m_pVertexDecl.mPtr );

    UINT stride = sizeof( Simple2DVertex );
    UINT offset = 0;
    context->BindVertexBuffers( {m_pVertexBuffer.mPtr}, &stride, &offset );

    context->SetPrimitiveTopology( prim );
    context->BindShader( m_pBaseVS.mPtr );
    if ( m_pTextureContext ) {
        Sampler sampler{};
        sampler.adressU = SamplerAddressing::Wrap;
        sampler.adressV = SamplerAddressing::Wrap;
        sampler.adressW = SamplerAddressing::Wrap;
        sampler.filtering = SamplerFilter::Linear;
        sampler.comparison = ComparisonFunc::Never;
        sampler.borderColor = {0xFF, 0xFF, 0xFF, 0xFF};
        context->BindSamplers( {{0, &sampler}}, ShaderStage::Pixel );

        context->BindImageBuffers( ImageBindType::PSResource, {{0, m_pTextureContext}} );
        context->BindShader( m_pTexPS.mPtr );
    } else {
        context->BindImageBuffers( ImageBindType::PSResource, {{0, nullptr}} );
        context->BindShader( m_pNoTexPS.mPtr );
    }
    // draw primitive
    context->FlushCache();
    context->RecordDrawCall( vertexCount, 0 );
    context->BindImageBuffers( ImageBindType::PSResource, {{0, nullptr}} );
    context->FlushCache();
}

void rh::engine::D3D11Im2DPipeline::DrawIndexed( void *impl,
                                               PrimitiveType prim,
                                               Simple2DVertex *vertices,
                                               unsigned int numVertices,
                                               VertexIndex *indices,
                                               unsigned int numIndices )
{
    auto *context = static_cast<IRenderingContext *>( impl );

    RH_ASSERT( numVertices > 0 )

    // dynamic vertex buffer update
    if ( prim != PrimitiveType::TriangleFan )
        context->UpdateBuffer( m_pVertexBuffer.mPtr,
                               vertices,
                               sizeof( Simple2DVertex ) * numVertices );
    else {
        rh::debug::DebugLogger::Error( "Trying to render trifan with indices!" );
        return;
    }
    context->UpdateBuffer( m_pIndexBuffer.mPtr, indices, sizeof( VertexIndex ) * numIndices );

    // initialize primitive info
    context->BindInputLayout( m_pVertexDecl.mPtr );

    UINT stride = sizeof( Simple2DVertex );
    UINT offset = 0;
    context->BindVertexBuffers( {m_pVertexBuffer.mPtr}, &stride, &offset );
    context->BindIndexBuffer( m_pIndexBuffer.mPtr );

    context->SetPrimitiveTopology( prim );
    context->BindShader( m_pBaseVS.mPtr );
    if ( m_pTextureContext ) {
        Sampler sampler{};
        sampler.adressU = SamplerAddressing::Wrap;
        sampler.adressV = SamplerAddressing::Wrap;
        sampler.adressW = SamplerAddressing::Wrap;
        sampler.filtering = SamplerFilter::Linear;
        sampler.comparison = ComparisonFunc::Never;
        sampler.borderColor = {0xFF, 0xFF, 0xFF, 0xFF};
        context->BindSamplers( {{0, &sampler}}, ShaderStage::Pixel );

        context->BindImageBuffers( ImageBindType::PSResource, {{0, m_pTextureContext}} );
        context->BindShader( m_pTexPS.mPtr );
    } else {
        context->BindImageBuffers( ImageBindType::PSResource, {{0, nullptr}} );
        context->BindShader( m_pNoTexPS.mPtr );
    }
    // draw primitive
    context->FlushCache();
    context->RecordDrawCallIndexed( numIndices, 0, 0 );
    context->BindImageBuffers( ImageBindType::PSResource, {{0, nullptr}} );
    context->FlushCache();
}

void rh::engine::D3D11Im2DPipeline::BindTexture( void *texture )
{
    m_pTextureContext = reinterpret_cast<D3D11Texture2D *>( texture );
}
