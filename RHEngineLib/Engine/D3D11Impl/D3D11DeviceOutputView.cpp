#include "D3D11DeviceOutputView.h"
#include "D3D11Common.h"
#include "Engine/D3D11Impl/D3D11GPUAllocator.h"
#include "Engine/D3D11Impl/ImageBuffers/D3D11BackBuffer.h"

rh::engine::D3D11DeviceOutputView::D3D11DeviceOutputView( HWND window, IDXGISwapChain *swapChainPtr )
    : m_pSwapChain( swapChainPtr )
    , m_hWnd( window )
{}

rh::engine::D3D11DeviceOutputView::~D3D11DeviceOutputView()
{
    delete m_pBackBufferView;
    if ( m_pSwapChain != nullptr ) {
        m_pSwapChain->Release();
        m_pSwapChain = nullptr;
    }
}

bool rh::engine::D3D11DeviceOutputView::Present()
{
    return CALL_D3D_API( m_pSwapChain->Present( 0, 0 ),
                         TEXT( "Fatal failure: either your GPU was removed, or some other nasty "
                               "thing happend, please restart application" ) );
}

bool rh::engine::D3D11DeviceOutputView::Resize( IGPUAllocator *allocator,
                                                size_t height,
                                                size_t width )
{ /*
    RECT rc;
    GetClientRect( m_hWnd, &rc );

    auto windowWidth = static_cast<size_t>( rc.right - rc.left );
    auto windowHeight = static_cast<size_t>( rc.bottom - rc.top );

    if ( windowWidth == width && windowHeight == height )
        return true;
    if ( !SetWindowPos(
             m_hWnd, nullptr, 0, 0, static_cast<int>( width ), static_cast<int>( height ), 0 ) ) {
        rh::debug::DebugLogger::Error( TEXT( "Failure while changing window size." ) );
        return false;
    }*/
    DXGI_MODE_DESC desc;
    desc.Width = width;
    desc.Height = height;
    m_pSwapChain->ResizeTarget( &desc );

    if ( m_pBackBufferView )
        m_pBackBufferView->ReleaseViews();
    // TODO: RELEASE ALL BBuffs before resize, and recreate them after

    if ( !CALL_D3D_API( m_pSwapChain->ResizeBuffers( 0,
                                                     static_cast<UINT>( width ),
                                                     static_cast<UINT>( height ),
                                                     DXGI_FORMAT_UNKNOWN,
                                                     0 ),
                        TEXT( "Fatal failure while changing screen size" ) ) )
        return false;

    auto d3d_allocator = static_cast<D3D11GPUAllocator *>( allocator );
    if ( m_pBackBufferView )
        m_pBackBufferView->RecreateViews( d3d_allocator->GetDevice(), m_pSwapChain );
    return true;
}

bool rh::engine::D3D11DeviceOutputView::SetFullscreenFlag( bool flag )
{
    return CALL_D3D_API( m_pSwapChain->SetFullscreenState( flag, nullptr ),
                         TEXT( "Failed to set fullscreen state." ) );
}

rh::engine::D3D11BackBuffer *rh::engine::D3D11DeviceOutputView::GetBackBufferView(
    ID3D11Device *device )
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
    rh::engine::ISyncPrimitive *signal_prim )
{}

bool rh::engine::D3D11DeviceOutputView::Present( uint32_t swapchain_img,
                                                 rh::engine::ISyncPrimitive *waitFor )
{}

rh::engine::IImageView *rh::engine::D3D11DeviceOutputView::GetImageView( uint32_t id ) {}
