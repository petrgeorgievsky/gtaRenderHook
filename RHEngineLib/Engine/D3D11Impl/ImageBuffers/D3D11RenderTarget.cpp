#include "D3D11RenderTarget.h"
#include "../D3D11Common.h"
#include <common.h>

rh::engine::D3D11BackBuffer::D3D11BackBuffer( ID3D11Device *device, IDXGISwapChain *swapChain )
{
    ID3D11Texture2D *backBufferTexture;

    if ( !CALL_D3D_API( swapChain->GetBuffer( 0,
                                              __uuidof( ID3D11Texture2D ),
                                              reinterpret_cast<void **>( &backBufferTexture ) ),
                        TEXT( "Failed to get back buffer texture." ) ) )
        return;

    if ( backBufferTexture == nullptr )
        return;

    if ( !CALL_D3D_API( device->CreateRenderTargetView( backBufferTexture,
                                                        nullptr,
                                                        &m_pRenderTargetView ),
                        TEXT( "Failed to create render target view for backbuffer texture." ) ) )
        return;

    backBufferTexture->Release();
}

rh::engine::D3D11BackBuffer::~D3D11BackBuffer()
{
    if ( m_pRenderTargetView ) {
        m_pRenderTargetView->Release();
        m_pRenderTargetView = nullptr;
    }
}

ID3D11RenderTargetView *rh::engine::D3D11BackBuffer::GetRenderTargetView()
{
    return m_pRenderTargetView;
}
