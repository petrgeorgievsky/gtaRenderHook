#include "D3D11Pipeline.h"
#include "D3D11Common.h"
#include "D3D11Convert.h"
#include "D3D11Shader.h"
#include "DebugUtils/DebugLogger.h"
#include <cassert>
#include <d3d11.h>

using namespace rh::engine;

constexpr D3D11_PRIMITIVE_TOPOLOGY Convert( Topology el_type )
{
    switch ( el_type )
    {
    case Topology::TriangleList:
        return D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    case Topology::LineList:
        return D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
    case Topology::PointList:
        return D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
    }
}

D3D11Pipeline::D3D11Pipeline(
    const D3D11GraphicsPipelineCreateInfo &create_info )
{
    ID3DBlob *vs_blob = nullptr;
    // fill shader binding table
    for ( auto shader_stage : create_info.mShaderStages )
    {
        auto shader = static_cast<ID3D11DeviceChild *>(
            *static_cast<D3D11Shader *>( shader_stage.mShader ) );
        if ( shader_stage.mStage == ShaderStage::Vertex )
        {
            // fill vertex shader ptr
            mShaderBindTable.mVertexShader =
                static_cast<ID3D11VertexShader *>( shader );
            mShaderBindTable.mVertexShader->AddRef();
            // extract vs_blob
            vs_blob = static_cast<ID3DBlob *>(
                *static_cast<D3D11Shader *>( shader_stage.mShader ) );
        }
        else if ( shader_stage.mStage == ShaderStage::Pixel )
        {
            // fill pixel shader ptr
            mShaderBindTable.mPixelShader =
                static_cast<ID3D11PixelShader *>( shader );
            mShaderBindTable.mPixelShader->AddRef();
        }
    }
    if ( vs_blob == nullptr )
    {
        debug::DebugLogger::Error(
            "Incorrect pipeline: vertex shader module is missing!" );
        return;
    }

    std::vector<D3D11_INPUT_ELEMENT_DESC> input_elements;
    for ( const auto &desc :
          create_info.mVertexInputStateDesc.mVertexInputLayout )
    {
        auto binding = create_info.mVertexInputStateDesc
                           .mVertexBindingLayout[desc.mBinding];

        D3D11_INPUT_ELEMENT_DESC impl_desc{};

        impl_desc.Format = GetVertexFormat( desc.mFormat ).first;

        impl_desc.InputSlotClass =
            binding.mInputRate == VertexBindingRate::PerInstance
                ? D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_INSTANCE_DATA
                : D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
        impl_desc.InputSlot         = desc.mBinding;
        impl_desc.AlignedByteOffset = desc.mOffset;
        impl_desc.SemanticName      = desc.mSemantics.c_str();
        impl_desc.SemanticIndex     = desc.mSemanticsId;
        input_elements.push_back( impl_desc );
    }

    create_info.mDevice->CreateInputLayout(
        input_elements.data(), input_elements.size(),
        vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), &mInputLayout );
    mTopology = Convert( create_info.mTopology );

    CD3D11_RASTERIZER_DESC rasterizer_desc( CD3D11_DEFAULT{} );
    rasterizer_desc.FrontCounterClockwise = true;
    rasterizer_desc.CullMode              = D3D11_CULL_NONE;

    auto result = create_info.mDevice->CreateRasterizerState(
        &rasterizer_desc, &mRasterizerState );
    assert( result == S_OK );

    D3D11_BLEND_DESC blend_desc{};
    blend_desc.RenderTarget[0].BlendOp        = D3D11_BLEND_OP_ADD;
    blend_desc.RenderTarget[0].BlendOpAlpha   = D3D11_BLEND_OP_ADD;
    blend_desc.RenderTarget[0].SrcBlend       = D3D11_BLEND_SRC_ALPHA;
    blend_desc.RenderTarget[0].SrcBlendAlpha  = D3D11_BLEND_SRC_ALPHA;
    blend_desc.RenderTarget[0].DestBlend      = D3D11_BLEND_INV_SRC_ALPHA;
    blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    blend_desc.RenderTarget[0].BlendEnable    = true;
    result = create_info.mDevice->CreateBlendState( &blend_desc, &mBlendState );
    assert( result == S_OK );
}

D3D11Pipeline::~D3D11Pipeline() {}

void D3D11Pipeline::BindToContext( ID3D11DeviceContext *context )
{
    context->IASetInputLayout( mInputLayout );
    context->IASetPrimitiveTopology( mTopology );
    context->VSSetShader( mShaderBindTable.mVertexShader, nullptr, 0 );
    context->PSSetShader( mShaderBindTable.mPixelShader, nullptr, 0 );
    context->RSSetState( mRasterizerState );
    context->OMSetBlendState( mBlendState, nullptr, 0 );
}
