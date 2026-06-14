#pragma once
#include <cstdint>

namespace rh::engine
{
class ISwapchain;

enum class WindowFlags : uint32_t
{
    NONE       = 0x0,
    FULLSCREEN = 0x1,
    HDR_OUTPUT = 0x2,
};

inline uint32_t operator&( uint32_t lhs, WindowFlags rhs )
{
    return static_cast<uint32_t>( rhs ) & lhs;
}

struct WindowParams
{
    uint32_t mWidth;
    uint32_t mHeight;
    uint32_t mFlags = static_cast<uint32_t>( WindowFlags::NONE );
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
    virtual ~IWindow()                    = default;
    IWindow()                             = default;
    IWindow( const IWindow & )            = delete;
    IWindow &operator=( const IWindow & ) = delete;
    IWindow( IWindow && )                 = delete;
    IWindow &operator=( IWindow && )      = delete;

    virtual bool SetWindowParams( const WindowParams &params ) = 0;

    virtual const WindowParams &GetWindowParams() = 0;

    virtual SwapchainRequestResult GetSwapchain() = 0;
};

} // namespace rh::engine