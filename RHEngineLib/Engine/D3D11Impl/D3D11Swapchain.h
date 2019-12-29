#pragma once
#include "Engine/Common/ISwapchain.h"
#include <common.h>

namespace rh::engine
{
struct D3D11SwapchainCreateParams
{
    // Dependencies...
    HWND          mWindowHandle;
    ID3D11Device *mDevice;
    IDXGIFactory *mDXGIFactory;
    // Present params
    PresentationParams mPresentParams;
};

class D3D11Swapchain : public ISwapchain
{
  public:
    D3D11Swapchain( const D3D11SwapchainCreateParams &create_params );
    ~D3D11Swapchain() override;
    SwapchainFrame GetAvaliableFrame( ISyncPrimitive *signal ) override;
    bool           Present( SwapchainFrame &swapchain_img,
                            ISyncPrimitive *waitFor ) override;

  private:
    uint32_t         mWidth          = 0;
    uint32_t         mHeight         = 0;
    IDXGISwapChain * mSwapchain      = nullptr;
    ID3D11Texture2D *mBackbufer      = nullptr;
    IImageView *     mBackbufferView = nullptr;
};
} // namespace rh::engine