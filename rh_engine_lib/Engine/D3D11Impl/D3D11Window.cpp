#include "D3D11Window.h"
#include "D3D11Swapchain.h"
#include <Windows.h>

using namespace rh::engine;

D3D11Window::D3D11Window( const D3D11WindowCreateParams &params )
{
    mWndHandle   = params.mWndHandle;
    mDevice      = params.mDevice;
    mDXGIFactory = params.mDXGIFactory;

    SetWindowParamsImpl( params.mWindowParams );
}

D3D11Window::~D3D11Window() { delete mSwapchain; }

bool D3D11Window::SetWindowParams( const WindowParams &params )
{
    return SetWindowParamsImpl( params );
}

SwapchainRequestResult D3D11Window::GetSwapchain()
{
    SwapchainRequestResult res{};
    if ( mIsSwapchainValid && mSwapchain )
    {
        res.mSwapchain = mSwapchain;

        return res;
    }

    if ( mSwapchain )
    {
        delete mSwapchain;
        mSwapchain = nullptr;
    }

    // Setup window params
    RECT rect;
    rect.top    = 0;
    rect.left   = 0;
    rect.right  = static_cast<LONG>( mCurrentParams.mWidth );
    rect.bottom = static_cast<LONG>( mCurrentParams.mHeight );

    auto wnd_ex_style = GetWindowLongA( mWndHandle, GWL_EXSTYLE );
    auto wnd_style    = GetWindowLongA( mWndHandle, GWL_STYLE );
    auto wnd_has_menu = GetMenu( mWndHandle ) != nullptr;
    // TODO: Allow to change
    auto wnd_flags = ( SWP_NOMOVE | SWP_NOZORDER );

    AdjustWindowRectEx( &rect, static_cast<DWORD>( wnd_style ), wnd_has_menu,
                        static_cast<DWORD>( wnd_ex_style ) );

    SetWindowPos( mWndHandle, nullptr, rect.left, rect.top,
                  rect.right - rect.left, rect.bottom - rect.top, wnd_flags );

    // Create new swap-chain
    D3D11SwapchainCreateParams sw_create_params{};
    sw_create_params.mWindowHandle            = mWndHandle;
    sw_create_params.mDXGIFactory             = mDXGIFactory;
    sw_create_params.mDevice                  = mDevice;
    sw_create_params.mPresentParams.mWindowed = !mCurrentParams.mFullscreen;

    res.mSwapchain    = ( mSwapchain = new D3D11Swapchain( sw_create_params ) );
    res.mChanged      = true;
    mIsSwapchainValid = true;

    return res;
}

bool D3D11Window::SetWindowParamsImpl( const WindowParams &params )
{
    mIsSwapchainValid = mCurrentParams.mWidth == params.mWidth &&
                        mCurrentParams.mHeight == params.mHeight;
    mCurrentParams = params;
    return true;
}

const WindowParams &D3D11Window::GetWindowParams() { return mCurrentParams; }