#pragma once
#include <cstdint>

namespace rh::engine
{
class ISwapchain;

struct WindowParams
{
    uint32_t mWidth;
    uint32_t mHeight;
    uint32_t mFullscreen;
    uint32_t mPadd;
};

struct SwapchainRequestResult
{
    ISwapchain *mSwapchain;
    bool        mChanged;
};

/**
 * @brief Represents native window
 *
 */
class IWindow
{
  public:
    virtual ~IWindow()         = default;
    IWindow()                  = default;
    IWindow( const IWindow & ) = delete;
    IWindow &operator=( const IWindow & ) = delete;
    IWindow( IWindow && )                 = delete;
    IWindow &operator=( IWindow && ) = delete;

    virtual bool SetWindowParams( const WindowParams &params ) = 0;

    virtual const WindowParams &GetWindowParams() = 0;

    virtual SwapchainRequestResult GetSwapchain() = 0;
};
} // namespace rh::engine