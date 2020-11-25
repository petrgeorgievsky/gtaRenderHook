#include "D3D11DeviceOutputView.h"
#include "D3D11Common.h"
#include "Engine/D3D11Impl/ImageBuffers/D3D11BackBuffer.h"
#include <d3d11_3.h>

rh::engine::D3D11DeviceOutputView::D3D11DeviceOutputView(
    HWND window, IDXGISwapChain *swapChainPtr )
    : m_pSwapChain( swapChainPtr ), m_hWnd( window )
{
}

rh::engine::D3D11DeviceOutputView::~D3D11DeviceOutputView()
{
    delete m_pBackBufferView;
    if ( m_pSwapChain != nullptr )
    {
        m_pSwapChain->Release();
        m_pSwapChain = nullptr;
    }
}

bool rh::engine::D3D11DeviceOutputView::Present()
{
    return CALL_D3D_API(
        m_pSwapChain->Present( 0, 0 ),
        TEXT( "Fatal failure: either your GPU was removed, or some other nasty "
              "thing happend, please restart application" ) );
}

bool rh::engine::D3D11DeviceOutputView::SetFullscreenFlag( bool flag )
{
    return CALL_D3D_API( m_pSwapChain->SetFullscreenState( flag, nullptr ),
                         TEXT( "Failed to set fullscreen state." ) );
}

rh::engine::D3D11BackBuffer *
rh::engine::D3D11DeviceOutputView::GetBackBufferView( ID3D11Device *device )
{
    if ( m_pBackBufferView == nullptr )
        m_pBackBufferView = new D3D11BackBuffer( device, m_pSwapChain );
    return m_pBackBufferView;
}

IDXGISwapChain *rh::engine::D3D11DeviceOutputView::GetSwapChainImpl()
{
    return m_pSwapChain;
}

uint32_t rh::engine::D3D11DeviceOutputView::GetFreeSwapchainImage(
    rh::engine::ISyncPrimitive * /*signal_prim */ )
{
    return 0;
}

bool rh::engine::D3D11DeviceOutputView::Present(
    uint32_t /*swapchain_img*/, rh::engine::ISyncPrimitive * /*waitFor */ )
{
    return false;
}

rh::engine::IImageView *
    rh::engine::D3D11DeviceOutputView::GetImageView( uint32_t /*id*/ )
{
    return nullptr;
}
