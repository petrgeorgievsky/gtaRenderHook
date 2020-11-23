#include "D3D11Texture2D.h"
#include "Engine/D3D11Impl/D3D11Common.h"
#include <d3d11_3.h>
using namespace rh::engine;

D3D11Texture2D::D3D11Texture2D( ID3D11Device *device,
                                const D3D11Texture2DCreateInfo &createInfo,
                                std::vector<D3D11_SUBRESOURCE_DATA> &&initialData )
{
    m_createInfo = createInfo;
    Load( device, std::move( initialData ) );
}

void D3D11Texture2D::Load( ID3D11Device *device, std::vector<D3D11_SUBRESOURCE_DATA> &&initialData )
{
    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = m_createInfo.width;
    desc.Height = m_createInfo.height;
    desc.Format = m_createInfo.format;
    desc.ArraySize = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = 0;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.MipLevels = m_createInfo.mipCount;

    if ( m_createInfo.allowShaderUsage )
        desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
    if ( m_createInfo.allowUnorderedAccessUsage )
        desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
    if ( m_createInfo.allowRenderTargetUsage )
        desc.BindFlags |= D3D11_BIND_RENDER_TARGET;

    if ( !CALL_D3D_API( device->CreateTexture2D( &desc, initialData.data(), &m_pTexture ),
                        TEXT( "Failed to create 2D texture" ) ) )
        return;

    if ( m_createInfo.allowShaderUsage ) {
        D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc{};
        srv_desc.Format = m_createInfo.format;
        srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srv_desc.Texture2D.MipLevels = UINT32_MAX;
        if ( !CALL_D3D_API( device->CreateShaderResourceView( m_pTexture,
                                                              &srv_desc,
                                                              &m_pShaderResourceView ),
                            TEXT( "Failed to create shader resource view over 2D texture" ) ) )
            return;
    }

    if ( m_createInfo.allowUnorderedAccessUsage ) {
        D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc{};
        uav_desc.Format = m_createInfo.format;
        uav_desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
        if ( !CALL_D3D_API( device->CreateUnorderedAccessView( m_pTexture,
                                                               &uav_desc,
                                                               &m_pUnorderedAccessView ),
                            TEXT( "Failed to create unordered access view over 2D texture" ) ) )
            return;
    }

    if ( m_createInfo.allowRenderTargetUsage ) {
        D3D11_RENDER_TARGET_VIEW_DESC rtv_desc{};
        rtv_desc.Format = m_createInfo.format;
        rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        if ( !CALL_D3D_API( device->CreateRenderTargetView( m_pTexture,
                                                            &rtv_desc,
                                                            &m_pRenderTargetView ),
                            TEXT( "Failed to create render target view over 2D texture" ) ) )
            return;
    }
    m_bLoaded = true;
}

D3D11Texture2D::~D3D11Texture2D()
{
    if ( m_pTexture ) {
        m_pTexture->Release();
        m_pTexture = nullptr;
    }
    if ( m_pShaderResourceView ) {
        m_pShaderResourceView->Release();
        m_pShaderResourceView = nullptr;
    }
    if ( m_pUnorderedAccessView ) {
        m_pUnorderedAccessView->Release();
        m_pUnorderedAccessView = nullptr;
    }
    if ( m_pRenderTargetView ) {
        m_pRenderTargetView->Release();
        m_pRenderTargetView = nullptr;
    }
}

ID3D11RenderTargetView *D3D11Texture2D::GetRenderTargetView()
{
    return m_pRenderTargetView;
}

ID3D11UnorderedAccessView *D3D11Texture2D::GetUnorderedAccessView()
{
    return m_pUnorderedAccessView;
}

ID3D11ShaderResourceView *D3D11Texture2D::GetShaderResourceView()
{
    return m_pShaderResourceView;
}

const D3D11Texture2DCreateInfo &D3D11Texture2D::GetTextureInfo() const
{
    return m_createInfo;
}

void *D3D11Texture2D::GetImplResource()
{
    return m_pTexture;
}
