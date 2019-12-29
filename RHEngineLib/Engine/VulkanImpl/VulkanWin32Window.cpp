#include "VulkanWin32Window.h"
#include "VulkanSwapchain.h"

using namespace rh::engine;

VulkanWin32Window::VulkanWin32Window(
    const VulkanWin32WindowCreateParams &params )
    : mWndHandle( params.mWndHandle ), mGPU( params.mGPU ),
      mInstance( params.mInstance ),
      mPresentQueueIdx( params.mPresentQueueIdx ),
      mPresentQueue( params.mPresentQueue ), mDevice( params.mDevice )
{
    vk::Win32SurfaceCreateInfoKHR window_info{};
    window_info.hinstance = GetModuleHandle( nullptr );
    window_info.hwnd      = mWndHandle;
    mSurface              = mInstance.createWin32SurfaceKHR( window_info );

    if ( !mGPU.getSurfaceSupportKHR( mPresentQueueIdx, mSurface ) )
    {
        mInstance.destroySurfaceKHR( mSurface );
        throw std::runtime_error(
            "Window surface can't be used with this queue!" );
    }
    SetWindowParams( params.mWindowParams );
}

VulkanWin32Window::~VulkanWin32Window()
{
    delete mSwapchain;
    mInstance.destroySurfaceKHR( mSurface );
}

bool VulkanWin32Window::SetWindowParams( const WindowParams &params )
{
    mIsSwapchainValid = mCurrentParams.mWidth == params.mWidth &&
                        mCurrentParams.mHeight == params.mHeight;
    mCurrentParams = params;
    return true;
}

SwapchainRequestResult VulkanWin32Window::GetSwapchain()
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

    VulkanSwapchainCreateParams vksc_cp{};
    vksc_cp.mGPU             = mGPU;
    vksc_cp.mDevice          = mDevice;
    vksc_cp.mPresentQueue    = mPresentQueue;
    vksc_cp.mPresentQueueIdx = mPresentQueueIdx;
    vksc_cp.mSurface         = mSurface;
    // Create new swap-chain
    res.mSwapchain    = ( mSwapchain = new VulkanSwapchain( vksc_cp ) );
    res.mChanged      = true;
    mIsSwapchainValid = true;

    return res;
}

const WindowParams &VulkanWin32Window::GetWindowParams()
{
    return mCurrentParams;
}