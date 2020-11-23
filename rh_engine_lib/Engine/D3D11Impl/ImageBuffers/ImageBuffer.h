#pragma once
#include "Engine/Common/IGPUResource.h"
struct ID3D11RenderTargetView;
struct ID3D11UnorderedAccessView;
struct ID3D11DepthStencilView;
struct ID3D11ShaderResourceView;
struct ID3D11Resource;

namespace rh::engine {

class D3D11BindableResource : public IGPUResource
{
public:
    ~D3D11BindableResource() override = default;

    /**
   * @brief Get the Render Target View object
   *
   * @return ID3D11RenderTargetView* - pointer to created render target view
   */
    virtual ID3D11RenderTargetView *GetRenderTargetView() { return nullptr; }

    /**
   * @brief Get the Unordered Access View object
   *
   * @return ID3D11UnorderedAccessView* - pointer to created unordered access
   * view
   */
    virtual ID3D11UnorderedAccessView *GetUnorderedAccessView() { return nullptr; }

    /**
   * @brief Get the Depth Stencil View object
   *
   * @return ID3D11DepthStencilView* - pointer to created depth stencil view
   */
    virtual ID3D11DepthStencilView *GetDepthStencilView() { return nullptr; }

    /**
   * @brief Get the Shader Resource View object
   *
   * @return ID3D11ShaderResourceView* - pointer to created shader resource view
   */
    virtual ID3D11ShaderResourceView *GetShaderResourceView() { return nullptr; }
};

} // namespace rh::engine
