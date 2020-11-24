#include "D3D11Convert.h"
#include "Engine/Common/types/blend_op.h"
#include "Engine/Common/types/comparison_func.h"
#include "Engine/Common/types/image_buffer_format.h"
#include "Engine/Common/types/input_element_type.h"
#include "Engine/Common/types/stencil_op.h"
#include <d3d11.h>

namespace rh::engine
{

DXGI_FORMAT GetDXGIResourceFormat( ImageBufferFormat format )
{
    switch ( format )
    {
    case ImageBufferFormat::Unknown: return DXGI_FORMAT_UNKNOWN;
    case ImageBufferFormat::BC1: return DXGI_FORMAT_BC1_UNORM;
    case ImageBufferFormat::BC2: return DXGI_FORMAT_BC2_UNORM;
    case ImageBufferFormat::BC3: return DXGI_FORMAT_BC3_UNORM;
    case ImageBufferFormat::BC4: return DXGI_FORMAT_BC4_UNORM;
    case ImageBufferFormat::BC5: return DXGI_FORMAT_BC5_UNORM;
    case ImageBufferFormat::BC6H: return DXGI_FORMAT_BC6H_SF16;
    case ImageBufferFormat::BC7: return DXGI_FORMAT_BC7_UNORM;
    case ImageBufferFormat::RGBA32: return DXGI_FORMAT_R32G32B32A32_FLOAT;
    case ImageBufferFormat::RGB32: return DXGI_FORMAT_R32G32B32_FLOAT;
    case ImageBufferFormat::RGBA16: return DXGI_FORMAT_R16G16B16A16_FLOAT;
    case ImageBufferFormat::RGB10A2: return DXGI_FORMAT_R10G10B10A2_UNORM;
    case ImageBufferFormat::RG11B10: return DXGI_FORMAT_R11G11B10_FLOAT;
    case ImageBufferFormat::RGBA8: return DXGI_FORMAT_R8G8B8A8_UNORM;
    case ImageBufferFormat::RG32: return DXGI_FORMAT_R32G32_FLOAT;
    case ImageBufferFormat::RG16: return DXGI_FORMAT_R16G16_FLOAT;
    case ImageBufferFormat::RG8: return DXGI_FORMAT_R8G8_UNORM;
    case ImageBufferFormat::R32G8: return DXGI_FORMAT_R32G8X24_TYPELESS;
    case ImageBufferFormat::R32: return DXGI_FORMAT_R32_FLOAT;
    case ImageBufferFormat::R16: return DXGI_FORMAT_R16_UNORM;
    case ImageBufferFormat::R8: return DXGI_FORMAT_R8_UNORM;
    case ImageBufferFormat::R8Uint: return DXGI_FORMAT_R8_UINT;
    case ImageBufferFormat::B5G6R5: return DXGI_FORMAT_B5G6R5_UNORM;
    case ImageBufferFormat::BGR5A1: return DXGI_FORMAT_B5G5R5A1_UNORM;
    case ImageBufferFormat::BGRA8: return DXGI_FORMAT_B8G8R8A8_UNORM;
    case ImageBufferFormat::BGR8: return DXGI_FORMAT_B8G8R8X8_UNORM;
    case ImageBufferFormat::BGRA4: return DXGI_FORMAT_B4G4R4A4_UNORM;
    case ImageBufferFormat::A8: return DXGI_FORMAT_A8_UNORM;
    case ImageBufferFormat::D24S8: return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
    }
    return DXGI_FORMAT_UNKNOWN;
}

String GetD3DShaderPrefix()
{
    /*auto *renderer = reinterpret_cast<D3D11Renderer *>( g_pRHRenderer.get() );
    switch ( renderer->GetFeatureLevel() )
    {
    case D3D_FEATURE_LEVEL_9_1: return TEXT( "4_0_level_9_1" );
    case D3D_FEATURE_LEVEL_9_2: return TEXT( "4_0_level_9_2" );
    case D3D_FEATURE_LEVEL_9_3: return TEXT( "4_0_level_9_3" );
    case D3D_FEATURE_LEVEL_10_0: return TEXT( "4_0" );
    case D3D_FEATURE_LEVEL_10_1: return TEXT( "4_1" );
    case D3D_FEATURE_LEVEL_11_0: return TEXT( "5_0" );
    case D3D_FEATURE_LEVEL_11_1: return TEXT( "5_0" );
    case D3D_FEATURE_LEVEL_12_0: return TEXT( "5_0" );
    case D3D_FEATURE_LEVEL_12_1: return TEXT( "5_0" );
    }*/
    return "5_0";
}

std::pair<DXGI_FORMAT, uint32_t> GetVertexFormat( InputElementType format )
{
    switch ( format )
    {
    case InputElementType::Float:
        return std::make_pair( DXGI_FORMAT_R32_FLOAT, 4 );
    case InputElementType::Vec2fp32:
        return std::make_pair( DXGI_FORMAT_R32G32_FLOAT, 8 );
    case InputElementType::Vec3fp32:
        return std::make_pair( DXGI_FORMAT_R32G32B32_FLOAT, 12 );
    case InputElementType::Vec4fp32:
        return std::make_pair( DXGI_FORMAT_R32G32B32A32_FLOAT, 16 );
    case InputElementType::Vec2fp16:
        return std::make_pair( DXGI_FORMAT_R16G16_FLOAT, 4 );
    case InputElementType::Vec4fp16:
        return std::make_pair( DXGI_FORMAT_R16G16B16A16_FLOAT, 8 );
    case InputElementType::Vec2fp8:
        return std::make_pair( DXGI_FORMAT_R8G8_UNORM, 2 );
    case InputElementType::Vec4fp8:
        return std::make_pair( DXGI_FORMAT_R8G8B8A8_UNORM, 4 );
    case InputElementType::Uint32:
        return std::make_pair( DXGI_FORMAT_R32_UINT, 4 );
    case InputElementType::Unknown: break;
    }
    return std::make_pair( DXGI_FORMAT_UNKNOWN, 0 );
}

D3D11_COMPARISON_FUNC
GetD3D11ComparisonFunc( const ComparisonFunc &comp_fn )
{
    switch ( comp_fn )
    {
    case ComparisonFunc::Always: return D3D11_COMPARISON_ALWAYS;
    case ComparisonFunc::Equal: return D3D11_COMPARISON_EQUAL;
    case ComparisonFunc::Greater: return D3D11_COMPARISON_GREATER;
    case ComparisonFunc::GreaterEqual: return D3D11_COMPARISON_GREATER_EQUAL;
    case ComparisonFunc::Less: return D3D11_COMPARISON_LESS;
    case ComparisonFunc::LessEqual: return D3D11_COMPARISON_LESS_EQUAL;
    case ComparisonFunc::Never: return D3D11_COMPARISON_NEVER;
    case ComparisonFunc::NotEqual: return D3D11_COMPARISON_NOT_EQUAL;
    case ComparisonFunc::Unknown: break;
    }
    return D3D11_COMPARISON_FUNC();
}

D3D11_STENCIL_OP GetD3D11StencilOp( const StencilOp &stencil_op )
{
    switch ( stencil_op )
    {
    case StencilOp::Decr: return D3D11_STENCIL_OP_DECR;
    case StencilOp::DecrSat: return D3D11_STENCIL_OP_DECR_SAT;
    case StencilOp::Incr: return D3D11_STENCIL_OP_INCR;
    case StencilOp::IncrSat: return D3D11_STENCIL_OP_INCR_SAT;
    case StencilOp::Invert: return D3D11_STENCIL_OP_INVERT;
    case StencilOp::Keep: return D3D11_STENCIL_OP_KEEP;
    case StencilOp::Replace: return D3D11_STENCIL_OP_REPLACE;
    case StencilOp::Zero: return D3D11_STENCIL_OP_ZERO;
    case StencilOp::Unknown: break;
    }
    abort();
}

D3D11_DEPTH_STENCIL_DESC
GetD3D11DepthStencilState( const DepthStencilState &desc )
{
    D3D11_DEPTH_STENCIL_DESC ds_desc{};
    // depth test parameters
    ds_desc.DepthEnable    = desc.enableDepthBuffer;
    ds_desc.DepthWriteMask = desc.enableDepthWrite
                                 ? D3D11_DEPTH_WRITE_MASK_ALL
                                 : D3D11_DEPTH_WRITE_MASK_ZERO;
    ds_desc.DepthFunc      = GetD3D11ComparisonFunc( desc.depthComparisonFunc );

    // stencil test parameters
    ds_desc.StencilEnable    = desc.enableStencilBuffer;
    ds_desc.StencilReadMask  = desc.stencilReadMask;
    ds_desc.StencilWriteMask = desc.stencilWriteMask;

    // stencil operations if pixel is front-facing
    ds_desc.FrontFace.StencilFailOp =
        GetD3D11StencilOp( desc.frontFaceStencilOp.stencilFailOp );
    ds_desc.FrontFace.StencilDepthFailOp =
        GetD3D11StencilOp( desc.frontFaceStencilOp.stencilDepthFailOp );
    ds_desc.FrontFace.StencilPassOp =
        GetD3D11StencilOp( desc.frontFaceStencilOp.stencilPassOp );
    ds_desc.FrontFace.StencilFunc =
        GetD3D11ComparisonFunc( desc.frontFaceStencilOp.stencilFunc );

    // stencil operations if pixel is back-facing
    ds_desc.BackFace.StencilFailOp =
        GetD3D11StencilOp( desc.backFaceStencilOp.stencilFailOp );
    ds_desc.BackFace.StencilDepthFailOp =
        GetD3D11StencilOp( desc.backFaceStencilOp.stencilDepthFailOp );
    ds_desc.BackFace.StencilPassOp =
        GetD3D11StencilOp( desc.backFaceStencilOp.stencilPassOp );
    ds_desc.BackFace.StencilFunc =
        GetD3D11ComparisonFunc( desc.backFaceStencilOp.stencilFunc );

    return ds_desc;
}

D3D11_BLEND GetD3D11BlendOp( const BlendOp &op )
{
    switch ( op )
    {
    case BlendOp::Zero: return D3D11_BLEND_ZERO;
    case BlendOp::One: return D3D11_BLEND_ONE;
    case BlendOp::SrcColor: return D3D11_BLEND_SRC_COLOR;
    case BlendOp::InvSrcColor: return D3D11_BLEND_INV_SRC_COLOR;
    case BlendOp::SrcAlpha: return D3D11_BLEND_SRC_ALPHA;
    case BlendOp::InvSrcAlpha: return D3D11_BLEND_INV_SRC_ALPHA;
    case BlendOp::DestAlpha: return D3D11_BLEND_DEST_ALPHA;
    case BlendOp::InvDestAlpha: return D3D11_BLEND_INV_DEST_ALPHA;
    case BlendOp::DestColor: return D3D11_BLEND_DEST_COLOR;
    case BlendOp::InvDestColor: return D3D11_BLEND_INV_DEST_COLOR;
    case BlendOp::SrcAlphaSat: return D3D11_BLEND_SRC_ALPHA_SAT;
    case BlendOp::BlendFactor: return D3D11_BLEND_BLEND_FACTOR;
    case BlendOp::Src1Color: return D3D11_BLEND_SRC1_COLOR;
    case BlendOp::InvSrc1Color: return D3D11_BLEND_INV_SRC1_COLOR;
    case BlendOp::Src1Alpha: return D3D11_BLEND_SRC1_ALPHA;
    case BlendOp::InvSrc1Alpha: return D3D11_BLEND_INV_SRC1_ALPHA;
    }
    return D3D11_BLEND_ZERO;
}

D3D11_BLEND_OP GetD3D11BlendCombineOp( const BlendCombineOp &op )
{
    switch ( op )
    {
    case BlendCombineOp::Add: return D3D11_BLEND_OP_ADD;
    case BlendCombineOp::Substract: return D3D11_BLEND_OP_SUBTRACT;
    case BlendCombineOp::RevSubstract: return D3D11_BLEND_OP_REV_SUBTRACT;
    case BlendCombineOp::Min: return D3D11_BLEND_OP_MIN;
    case BlendCombineOp::Max: return D3D11_BLEND_OP_MAX;
    }
    return D3D11_BLEND_OP_ADD;
}

D3D11_RENDER_TARGET_BLEND_DESC
GetD3D11PerRTBlendState( const AttachmentBlendState &desc )
{
    D3D11_RENDER_TARGET_BLEND_DESC desc_rt{};
    desc_rt.BlendEnable    = desc.enableBlending;
    desc_rt.BlendOp        = GetD3D11BlendCombineOp( desc.blendCombineOp );
    desc_rt.SrcBlend       = GetD3D11BlendOp( desc.srcBlend );
    desc_rt.DestBlend      = GetD3D11BlendOp( desc.destBlend );
    desc_rt.BlendOpAlpha   = GetD3D11BlendCombineOp( desc.blendAlphaCombineOp );
    desc_rt.SrcBlendAlpha  = GetD3D11BlendOp( desc.srcBlendAlpha );
    desc_rt.DestBlendAlpha = GetD3D11BlendOp( desc.destBlendAlpha );
    desc_rt.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    return desc_rt;
}

D3D11_BLEND_DESC GetD3D11BlendState( const BlendState &desc )
{
    D3D11_BLEND_DESC bl_desc{};
    for ( size_t i = 0; i < desc.renderTargetBlendState.size(); i++ )
    {
        bl_desc.RenderTarget[i] =
            GetD3D11PerRTBlendState( desc.renderTargetBlendState[i] );
    }
    return bl_desc;
}

D3D11_RASTERIZER_DESC
GetD3D11RasterizerState( const RasterizerState &desc )
{
    // TODO: add more params to RH state
    D3D11_RASTERIZER_DESC rs_desc{};
    rs_desc.AntialiasedLineEnable = false;
    rs_desc.DepthBias             = 0;
    rs_desc.DepthBiasClamp        = 0.0F;
    rs_desc.DepthClipEnable       = true;
    rs_desc.FillMode              = D3D11_FILL_SOLID;
    rs_desc.FrontCounterClockwise = true;
    rs_desc.MultisampleEnable     = true;
    rs_desc.ScissorEnable         = false;
    rs_desc.SlopeScaledDepthBias  = 0.0F;
    rs_desc.CullMode = static_cast<D3D11_CULL_MODE>( desc.cullMode );
    return rs_desc;
}

} // namespace rh::engine
