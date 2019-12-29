#include "stdafx.h"
#include "D3D1XCubeTexture.h"
#include "D3DSpecificHelpers.h"
#include "RwD3D1XEngine.h"
#include "D3D1XEnumParser.h"
#include "D3DRenderer.h"


CD3D1XCubeTexture::CD3D1XCubeTexture( RwRaster* parent, bool createMipMaps ) :
    CD3D1XBaseTexture( parent, eTextureDimension::TT_2D, "CubeTexture" )
{
    RwD3D1XRaster* d3dRaster = GetD3D1XRaster( m_pParent );
    ID3D11Device* dev = GET_D3D_DEVICE;

    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = parent->width;
    desc.Height = parent->height;
    desc.MipLevels = 1;
    desc.ArraySize = 6;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.CPUAccessFlags = 0;

    // Create the cube map for env map render target
    desc.Format = CD3D1XEnumParser::ConvertToTextureBufferSupportedFormat( d3dRaster->format );
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
    desc.MipLevels = createMipMaps ? max( (int)log2( min( desc.Width, desc.Height ) ) - 2, 0 ) : 1;
    if ( !CALL_D3D_API( dev->CreateTexture2D( &desc, nullptr,
                                              reinterpret_cast<ID3D11Texture2D**>( &m_pTextureResource ) ),
                        "Failed to create cubemap texture" ) )
        return;

    D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
    SRVDesc.Format = CD3D1XEnumParser::ConvertToSRVSupportedFormat( d3dRaster->format );
    SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    SRVDesc.TextureCube.MipLevels = createMipMaps ? max( (int)log2( min( desc.Width, desc.Height ) ) - 2, 0 ) : 1;
    SRVDesc.TextureCube.MostDetailedMip = 0;
    if ( !CALL_D3D_API( dev->CreateShaderResourceView( m_pTextureResource, &SRVDesc, &m_pShaderView ),
                        "Failed to create shader resource view for cubemap texture" ) )
        return;
}


CD3D1XCubeTexture::~CD3D1XCubeTexture()
{
    if ( m_pShaderView )
    {
        m_pShaderView->Release();
        m_pShaderView = nullptr;
    }
}
