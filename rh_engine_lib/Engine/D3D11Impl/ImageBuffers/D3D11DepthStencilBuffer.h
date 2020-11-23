#pragma once
#include "ImageBuffer.h"

#include <d3d11_3.h>
namespace rh::engine {
struct D3D11DepthStencilBufferCreateInfo
{
    unsigned int width;
    unsigned int height;
    DXGI_FORMAT format;
};

/**
 * @brief D3D11 Depth-Stencil Buffer implementation,
 * holds a Depth-Stencil view and it's texture buffer
 *
 */
class D3D11DepthStencilBuffer : public D3D11BindableResource
{
public:
    /**
   * @brief Construct a new D3D11DepthStencilBuffer object
   *
   * @param device - logical device used to allocate memory for graphics
   * resources
   */
    D3D11DepthStencilBuffer( ID3D11Device *device, const D3D11DepthStencilBufferCreateInfo &info );

    /**
   * @brief Destroy the D3D11DepthStencilBuffer object
   *
   */
    ~D3D11DepthStencilBuffer() override;

    /**
   * @brief Get the Render Target View object
   *
   * @return ID3D11RenderTargetView* - view of render target
   */
    ID3D11DepthStencilView *GetDepthStencilView() override;

    [[nodiscard]] const D3D11DepthStencilBufferCreateInfo &GetCreateInfo() const;

private:
    ID3D11Texture2D *m_pTexture = nullptr;
    ID3D11DepthStencilView *m_pDepthStencilView = nullptr;
    D3D11DepthStencilBufferCreateInfo m_createInfo;
};
} // namespace rh::engine
