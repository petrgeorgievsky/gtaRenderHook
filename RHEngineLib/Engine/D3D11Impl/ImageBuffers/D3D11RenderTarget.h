#pragma once
#include "ImageBuffer.h"
#include <d3d11_3.h>

namespace rh::engine {

/**
 * @brief D3D11 Back-buffer implementation,
 * holds a RenderTarget view of a swap-chain back buffer
 *
 */
class D3D11RenderTarget : public D3D11BindableResource
{
public:
    /**
   * @brief Construct a new D3D11BackBuffer object
   *
   * @param device - logical device used to allocate memory for graphics
   * resources
   * @param swapChain - swap-chain of this back-buffer
   */
    D3D11RenderTarget( ID3D11Device *device, IDXGISwapChain *swapChain );

    /**
   * @brief Destroy the D3D11BackBuffer object
   *
   */
    ~D3D11RenderTarget() override;

    /**
   * @brief Get the Render Target View object
   *
   * @return ID3D11RenderTargetView* - view of render target
   */
    ID3D11RenderTargetView *GetRenderTargetView() override;

private:
    ID3D11RenderTargetView *m_pRenderTargetView = nullptr;
};
}; // namespace rh::engine
