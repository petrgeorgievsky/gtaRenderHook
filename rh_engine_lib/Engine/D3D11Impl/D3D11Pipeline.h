#pragma once
#include "Engine/Common/IPipeline.h"
#include "d3dcommon.h"

// d3d11 struct forwards:
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11VertexShader;
struct ID3D11GeometryShader;
struct ID3D11HullShader;
struct ID3D11DomainShader;
struct ID3D11PixelShader;
struct ID3D11InputLayout;
struct ID3D11RasterizerState;
struct ID3D11BlendState;
enum D3D_PRIMITIVE_TOPOLOGY;
typedef D3D_PRIMITIVE_TOPOLOGY D3D11_PRIMITIVE_TOPOLOGY;

namespace rh::engine
{

struct D3D11PipelineShaders
{
    ID3D11VertexShader *  mVertexShader;
    ID3D11GeometryShader *mGeometryShader;
    ID3D11HullShader *    mHullShader;
    ID3D11DomainShader *  mDomainShader;
    ID3D11PixelShader *   mPixelShader;
};
struct D3D11GraphicsPipelineCreateInfo : RasterPipelineCreateParams
{
    ID3D11Device *mDevice;
};
class D3D11Pipeline : public IPipeline
{
  private:
    D3D11PipelineShaders     mShaderBindTable{};
    ID3D11InputLayout *      mInputLayout;
    D3D11_PRIMITIVE_TOPOLOGY mTopology;
    ID3D11RasterizerState *  mRasterizerState = nullptr;
    ID3D11BlendState *       mBlendState      = nullptr;

  public:
    D3D11Pipeline( const D3D11GraphicsPipelineCreateInfo &create_info );
    ~D3D11Pipeline() override;
    void BindToContext( ID3D11DeviceContext *context );
};

} // namespace rh::engine
