#include "D3D1X3DTexture.h"
#include "D3D1XEnumParser.h"
#include "D3DSpecificHelpers.h"
#include "RwD3D1XEngine.h"
#include "D3DRenderer.h"
#include "stdafx.h"

CD3D1X3DTexture::CD3D1X3DTexture(
    RwRaster *parent, const D3D11_SUBRESOURCE_DATA *resourse_data )
    : CD3D1XBaseTexture( parent, eTextureDimension::TT_3D, "3DTexture" )
{
    RwD3D1XRaster *d3dRaster = GetD3D1XRaster( m_pParent );
    ID3D11Device * dev       = GET_D3D_DEVICE;

    // Base texture 3D creation
    D3D11_TEXTURE3D_DESC desc{};
    desc.Width  = m_pParent->width;
    desc.Height = m_pParent->height;
    desc.Depth  = m_pParent->depth;
    desc.Format = CD3D1XEnumParser::ConvertToTextureBufferSupportedFormat(
        d3dRaster->format );
    desc.MipLevels          = 1;
    desc.Usage              = D3D11_USAGE_DEFAULT;
    desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags      = 0;


    if ( !CALL_D3D_API(
             dev->CreateTexture3D(
                 &desc, resourse_data,
                 reinterpret_cast<ID3D11Texture3D **>( &m_pTextureResource ) ),
             "Failed to create 2D texture" ) )
        return;
    if ( m_pTextureResource == nullptr )
        return;
    D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};

    SRVDesc.Format =
        CD3D1XEnumParser::ConvertToSRVSupportedFormat( d3dRaster->format );
    SRVDesc.ViewDimension       = D3D11_SRV_DIMENSION_TEXTURE3D;
    SRVDesc.Texture3D.MipLevels = UINT32_MAX;
    if ( !CALL_D3D_API(
             dev->CreateShaderResourceView( m_pTextureResource, &SRVDesc,
                                            &m_pShaderView ),
             "Failed to create shader resource view for 2D texture" ) )
        return;
}

CD3D1X3DTexture::~CD3D1X3DTexture() {

    if ( m_pShaderView )
    {
        m_pShaderView->Release();
        m_pShaderView = nullptr;
    }
}
