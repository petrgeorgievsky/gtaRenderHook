#pragma once
#include "Engine/Common/IWindow.h"
#include <common.h>

namespace rh::engine
{

struct D3D11WindowCreateParams
{
    // Dependencies...
    HWND          mWndHandle;
    ID3D11Device *mDevice;
    IDXGIFactory *mDXGIFactory;
    // Window params
    WindowParams mWindowParams;
};

class D3D11Window : public IWindow
{
  public:
    D3D11Window( const D3D11WindowCreateParams &params );
    ~D3D11Window();

    bool SetWindowParams( const WindowParams &params ) override;

    const WindowParams &GetWindowParams() override;

    SwapchainRequestResult GetSwapchain() override;

  private:
    WindowParams  mCurrentParams{};
    HWND          mWndHandle;
    ISwapchain *  mSwapchain        = nullptr;
    ID3D11Device *mDevice           = nullptr;
    IDXGIFactory *mDXGIFactory      = nullptr;
    bool          mIsSwapchainValid = false;
};
} // namespace rh::engine