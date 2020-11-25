#pragma once
#include "IImageView.h"
#include <cstdint>

namespace rh::engine
{
class ISyncPrimitive;

/**
 * @brief DeviceOutputView
 *
 * Represents native window and associated swap-chain
 * * Manages device-specific rendering output lifecycle
 * ! Doesn't own native window
 * TODO: Add comments
 */
class IDeviceOutputView
{
  public:
    virtual ~IDeviceOutputView()                   = default;
    IDeviceOutputView()                            = default;
    IDeviceOutputView( const IDeviceOutputView & ) = delete;
    IDeviceOutputView &operator=( const IDeviceOutputView & ) = delete;
    IDeviceOutputView( IDeviceOutputView && )                 = delete;
    IDeviceOutputView &operator=( IDeviceOutputView && ) = delete;

    virtual bool Present()                      = 0;
    virtual bool SetFullscreenFlag( bool flag ) = 0;

    virtual uint32_t GetFreeSwapchainImage( ISyncPrimitive *signal_prim ) = 0;

    virtual bool Present( uint32_t swapchain_img, ISyncPrimitive *waitFor ) = 0;
    virtual IImageView *GetImageView( uint32_t id )                         = 0;
};

} // namespace rh::engine
