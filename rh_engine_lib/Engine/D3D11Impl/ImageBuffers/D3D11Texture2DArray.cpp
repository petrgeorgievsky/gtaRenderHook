#include "D3D11Texture2DArray.h"
#include "Engine/D3D11Impl/D3D11Common.h"
using namespace rh::engine;

D3D11Texture2DArray::D3D11Texture2DArray( ID3D11Device *device,
                                          const D3D11Texture2DArrayCreateInfo &createInfo )
{
    m_createInfo = createInfo;
    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = m_createInfo.width;
    desc.Height = m_createInfo.height;
    desc.Format = m_createInfo.format;
    desc.ArraySize = m_createInfo.imageCount;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = 0;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.MipLevels = m_createInfo.mipCount;

    // std::vector<D3D11_SUBRESOURCE_DATA> data_vec;
    // data_vec.reserve( desc.MipLevels * desc.ArraySize );
    uint32_t bytesPerBlock = 16;
    switch ( m_createInfo.format ) {
    case DXGI_FORMAT_A8P8:
        bytesPerBlock = 4;
        break;
    case DXGI_FORMAT_B4G4R4A4_UNORM:
    case DXGI_FORMAT_B5G5R5A1_UNORM:
    case DXGI_FORMAT_B5G6R5_UNORM:
    case DXGI_FORMAT_BC1_UNORM:
    case DXGI_FORMAT_BC4_UNORM:
        bytesPerBlock = 8;
        break;
    default:
        bytesPerBlock = 16;
        break;
    }
    /*for ( uint32_t slice = 0; slice < desc.ArraySize; slice++ ) {
for ( auto mip = 0; mip < desc.MipLevels; mip++ ) {
  D3D11_SUBRESOURCE_DATA data{};
  uint32_t width = desc.Width >> mip;
  uint32_t height = desc.Height >> mip;
  data.SysMemPitch = bytesPerBlock * ( ( width + 3 ) / 4 );
  data.pSysMem = malloc( (height) *bytesPerBlock * ( ( width + 3 ) / 4 )
); data_vec.push_back( data );
}
}*/

    if ( m_createInfo.allowShaderUsage )
        desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
    if ( m_createInfo.allowUnorderedAccessUsage )
        desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
    if ( m_createInfo.allowRenderTargetUsage )
        desc.BindFlags |= D3D11_BIND_RENDER_TARGET;

    if ( !CALL_D3D_API( device->CreateTexture2D( &desc, nullptr, &m_pTexture ),
                        TEXT( "Failed to create 2D texture" ) ) )
        return;

    /*for ( auto data : data_vec ) {
free( const_cast<void *>( data.pSysMem ) );
}*/

    if ( m_createInfo.allowShaderUsage ) {
        D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc{};
        srv_desc.Format = m_createInfo.format;
        srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
        srv_desc.Texture2DArray.MipLevels = UINT32_MAX;
        srv_desc.Texture2DArray.ArraySize = m_createInfo.imageCount;
        if ( !CALL_D3D_API( device->CreateShaderResourceView( m_pTexture,
                                                              &srv_desc,
                                                              &m_pShaderResourceView ),
                            TEXT(
                                "Failed to create shader resource view over 2D texture array" ) ) )
            return;
    }

    if ( m_createInfo.allowUnorderedAccessUsage ) {
        D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc{};
        uav_desc.Format = m_createInfo.format;
        uav_desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
        if ( !CALL_D3D_API( device->CreateUnorderedAccessView( m_pTexture,
                                                               &uav_desc,
                                                               &m_pUnorderedAccessView ),
                            TEXT( "Failed to create unordered access view over 2D texture" ) ) )
            return;
    }

    if ( m_createInfo.allowRenderTargetUsage ) {
        D3D11_RENDER_TARGET_VIEW_DESC rtv_desc{};
        rtv_desc.Format = m_createInfo.format;
        rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
        if ( !CALL_D3D_API( device->CreateRenderTargetView( m_pTexture,
                                                            &rtv_desc,
                                                            &m_pRenderTargetView ),
                            TEXT( "Failed to create render target view over 2D texture" ) ) )
            return;
    }
    m_bLoaded = true;
}

D3D11Texture2DArray::~D3D11Texture2DArray()
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

ID3D11RenderTargetView *D3D11Texture2DArray::GetRenderTargetView()
{
    return m_pRenderTargetView;
}

ID3D11UnorderedAccessView *D3D11Texture2DArray::GetUnorderedAccessView()
{
    return m_pUnorderedAccessView;
}

ID3D11ShaderResourceView *D3D11Texture2DArray::GetShaderResourceView()
{
    return m_pShaderResourceView;
}

void *D3D11Texture2DArray::GetImplResource()
{
    return m_pTexture;
}
