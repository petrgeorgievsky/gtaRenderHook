#pragma once
#include <Engine/Common/types/blend_state.h>
#include <Engine/Common/types/depth_stencil_state.h>
#include <Engine/Common/types/rasterizer_state.h>
#include <Engine/Common/types/string_typedefs.h>
//
#include <utility>

// d3d11 forwards:
enum DXGI_FORMAT;
enum D3D11_COMPARISON_FUNC;
enum D3D11_STENCIL_OP;
enum D3D11_BLEND;
enum D3D11_BLEND_OP;
struct D3D11_DEPTH_STENCIL_DESC;
struct D3D11_RENDER_TARGET_BLEND_DESC;
struct D3D11_BLEND_DESC;
struct D3D11_RASTERIZER_DESC;

namespace rh::engine
{
enum class ImageBufferFormat : uint8_t;
enum class InputElementType : uint32_t;
DXGI_FORMAT GetDXGIResourceFormat( ImageBufferFormat format );

String GetD3DShaderPrefix();

std::pair<DXGI_FORMAT, uint32_t> GetVertexFormat( InputElementType format );

D3D11_COMPARISON_FUNC GetD3D11ComparisonFunc( const ComparisonFunc &comp_fn );

D3D11_STENCIL_OP GetD3D11StencilOp( const StencilOp &stencil_op );

D3D11_DEPTH_STENCIL_DESC
GetD3D11DepthStencilState( const DepthStencilState &desc );

D3D11_BLEND GetD3D11BlendOp( const BlendOp &op );

D3D11_BLEND_OP GetD3D11BlendCombineOp( const BlendCombineOp &op );

D3D11_RENDER_TARGET_BLEND_DESC
GetD3D11PerRTBlendState( const AttachmentBlendState &desc );

D3D11_BLEND_DESC GetD3D11BlendState( const BlendState &desc );

D3D11_RASTERIZER_DESC GetD3D11RasterizerState( const RasterizerState &desc );
} // namespace rh::engine
