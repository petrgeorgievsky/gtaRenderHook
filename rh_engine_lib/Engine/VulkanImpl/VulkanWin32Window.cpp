#include "VulkanWin32Window.h"
#include "VulkanSwapchain.h"
#include <DebugUtils/DebugLogger.h>

namespace rh::engine
{
VulkanWin32Window::VulkanWin32Window(
    const VulkanWin32WindowCreateParams &params )
    : mWndHandle( params.mWndHandle ), mInstance( params.mInstance ),
      mGPU( params.mGPU ), mDevice( params.mDevice ),
      mPresentQueue( params.mPresentQueue ),
      mPresentQueueIdx( params.mPresentQueueIdx )
{
    vk::Win32SurfaceCreateInfoKHR window_info{};
    window_info.hinstance = GetModuleHandle( nullptr );
    window_info.hwnd      = mWndHandle;
    auto create_result    = mInstance.createWin32SurfaceKHR( window_info );

    if ( create_result.result != vk::Result::eSuccess )
        debug::DebugLogger::ErrorFmt(
            "Failed to create win32 surface:%s",
            vk::to_string( create_result.result ).c_str() );
    else
        mSurface = create_result.value;

    auto support_result =
        mGPU.getSurfaceSupportKHR( mPresentQueueIdx, mSurface );
    if ( support_result.result != vk::Result::eSuccess )
    {
        debug::DebugLogger::ErrorFmt(
            "Failed to query surface support for queue %i :%s",
            mPresentQueueIdx, vk::to_string( support_result.result ).c_str() );
        std::terminate();
    }
    else if ( support_result.value != VK_TRUE )
    {
        mInstance.destroySurfaceKHR( mSurface );
        debug::DebugLogger::ErrorFmt(
            "Window surface is not supported by this gpu on queue %i",
            mPresentQueueIdx );
    }
    SetWindowParamsImpl( params.mWindowParams );
}

VulkanWin32Window::~VulkanWin32Window()
{
    delete mSwapchain;
    mInstance.destroySurfaceKHR( mSurface );
}

bool VulkanWin32Window::SetWindowParams( const WindowParams &params )
{
    return SetWindowParamsImpl( params );
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

    VulkanSwapchainCreateParams vksc_cp{};
    vksc_cp.mGPU                      = mGPU;
    vksc_cp.mDevice                   = mDevice;
    vksc_cp.mPresentQueue             = mPresentQueue;
    vksc_cp.mPresentQueueIdx          = mPresentQueueIdx;
    vksc_cp.mSurface                  = mSurface;
    vksc_cp.mPresentParams.mVsyncType = VSyncType::None;
    vksc_cp.mPresentParams.mUseHDR    = true;
    // vksc_cp.mPresentParams.mBufferCount = 3;
    // Create new swap-chain
    res.mSwapchain    = ( mSwapchain = new VulkanSwapchain( vksc_cp ) );
    res.mChanged      = true;
    mIsSwapchainValid = true;

    return res;
}

bool VulkanWin32Window::SetWindowParamsImpl( const WindowParams &params )
{
    mIsSwapchainValid = mCurrentParams.mWidth == params.mWidth &&
                        mCurrentParams.mHeight == params.mHeight;
    mCurrentParams = params;
    return true;
}

const WindowParams &VulkanWin32Window::GetWindowParams()
{
    return mCurrentParams;
}
} // namespace rh::engine