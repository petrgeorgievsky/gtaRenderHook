#pragma once
#include "Engine/Common/IDeviceOutputView.h"
#include <d3d11_3.h>
#include <vector>

namespace rh::engine {
class D3D11BackBuffer;
class D3D11DeviceOutputView : public IDeviceOutputView
{
public:
    D3D11DeviceOutputView( HWND window, IDXGISwapChain *swapChainPtr );
    ~D3D11DeviceOutputView() override;

    bool Present() override;

    bool Resize( IGPUAllocator *allocator, size_t height, size_t width ) override;

    bool SetFullscreenFlag( bool flag ) override;

    D3D11BackBuffer *GetBackBufferView( ID3D11Device *device );

    IDXGISwapChain *GetSwapChainImpl();

    uint32_t GetFreeSwapchainImage( ISyncPrimitive *signal_prim ) override;
    bool Present( uint32_t swapchain_img, ISyncPrimitive *waitFor ) override;
    IImageView *GetImageView( uint32_t id ) override;

private:
    /// Back-buffer swap-chain. Stores 2 or more image buffers,
    /// one being processed in this frame, and other from previous frames.
    IDXGISwapChain *m_pSwapChain = nullptr;

    /// Back-buffer swap-chain. Stores 2 or more image buffers,
    /// one being processed in this frame, and other from previous frames.
    HWND m_hWnd = nullptr;

    /// Back-buffer image view, for use as a RenderTarget
    D3D11BackBuffer *m_pBackBufferView = nullptr;
};

}; // namespace rh::engine
