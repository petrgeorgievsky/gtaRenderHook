#include "D3D11DepthStencilBuffer.h"
#include "Engine/D3D11Impl/D3D11Common.h"
using namespace rh::engine;

D3D11DepthStencilBuffer::D3D11DepthStencilBuffer( ID3D11Device *device,
                                                  const D3D11DepthStencilBufferCreateInfo &info )
    : m_createInfo( info )
{
    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = info.width;
    desc.Height = info.height;
    desc.Format = info.format;
    desc.ArraySize = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.MipLevels = 1;

    if ( !CALL_D3D_API( device->CreateTexture2D( &desc, nullptr, &m_pTexture ),
                        TEXT( "Failed to create depth stencil buffer" ) ) )
        return;

    D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc{};
    dsv_desc.Format = info.format;
    dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    if ( !CALL_D3D_API( device->CreateDepthStencilView( m_pTexture, &dsv_desc, &m_pDepthStencilView ),
                        TEXT( "Failed to create depth stencil view" ) ) )
        return;
}

D3D11DepthStencilBuffer::~D3D11DepthStencilBuffer()
{
    if ( m_pDepthStencilView ) {
        m_pDepthStencilView->Release();
        m_pDepthStencilView = nullptr;
    }
    if ( m_pTexture ) {
        m_pTexture->Release();
        m_pTexture = nullptr;
    }
}

ID3D11DepthStencilView *D3D11DepthStencilBuffer::GetDepthStencilView()
{
    return m_pDepthStencilView;
}

const D3D11DepthStencilBufferCreateInfo &D3D11DepthStencilBuffer::GetCreateInfo() const
{
    return m_createInfo;
}
