#pragma once
#include "Engine/Common/IWindow.h"
#include <common.h>

namespace rh::engine
{
class ISyncPrimitive;

struct VulkanWin32WindowCreateParams
{
    // Dependencies...
    HWND               mWndHandle;
    vk::Instance       mInstance;
    vk::PhysicalDevice mGPU;
    vk::Device         mDevice;
    vk::Queue          mPresentQueue;
    uint32_t           mPresentQueueIdx;
    // Window params
    WindowParams mWindowParams;
};

class VulkanWin32Window : public IWindow
{
  public:
    VulkanWin32Window( const VulkanWin32WindowCreateParams &params );
    virtual ~VulkanWin32Window() override;

    virtual bool SetWindowParams( const WindowParams &params ) override;
    virtual const WindowParams &   GetWindowParams() override;
    virtual SwapchainRequestResult GetSwapchain() override;

    HWND GetHandle() { return mWndHandle; }

  private:
    bool SetWindowParamsImpl( const WindowParams &params );

    HWND         mWndHandle;
    WindowParams mCurrentParams{};
    ISwapchain * mSwapchain        = nullptr;
    bool         mIsSwapchainValid = false;

    vk::Instance       mInstance;
    vk::PhysicalDevice mGPU;
    vk::Device         mDevice;
    vk::Queue          mPresentQueue;
    uint32_t           mPresentQueueIdx;
    vk::SurfaceKHR     mSurface;
};
} // namespace rh::engine