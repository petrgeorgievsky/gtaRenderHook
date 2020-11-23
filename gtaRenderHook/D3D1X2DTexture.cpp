// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "stdafx.h"
#include "D3D1X2DTexture.h"
#include "D3DSpecificHelpers.h"
#include "RwD3D1XEngine.h"
#include "D3D1XEnumParser.h"
#include "D3DRenderer.h"

CD3D1X2DTexture::CD3D1X2DTexture( RwRaster * parent, D3D11_BIND_FLAG bindFlags,
                                  const std::string & resourceTypeName, bool createMipMaps, bool shaderAccess, bool canReallocate ) :
    CD3D1XBaseTexture( parent, eTextureDimension::TT_2D, resourceTypeName ),
    m_bCanReallocate( canReallocate ), m_bHasMipMaps( createMipMaps ), m_dwBindFlags( bindFlags ), m_bShaderAccess( shaderAccess )
{
    RwD3D1XRaster* d3dRaster = GetD3D1XRaster( m_pParent );
    ID3D11Device* dev = GET_D3D_DEVICE;

    // Base texture 2D creation
    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = m_pParent->width;
    desc.Height = m_pParent->height;
    desc.Format = CD3D1XEnumParser::ConvertToTextureBufferSupportedFormat( d3dRaster->format );
    desc.ArraySize = 1;
    desc.MipLevels = createMipMaps ? max( (int)log2( min( desc.Width, desc.Height ) ) - 2, 0 ) : 1;
    desc.SampleDesc.Quality = 0;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = ( m_bShaderAccess ? D3D11_BIND_SHADER_RESOURCE : 0 ) | m_dwBindFlags;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    if ( !CALL_D3D_API( dev->CreateTexture2D( &desc, NULL,
                                              reinterpret_cast<ID3D11Texture2D**>( &m_pTextureResource ) ),
                        "Failed to create 2D texture" ) )
        return;
    // If texture has no shader access(e.g. depth stencil surface for camera), than we don't need shader resource view
    if ( !m_bShaderAccess || m_pTextureResource == nullptr )
        return;

    D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};

    SRVDesc.Format = CD3D1XEnumParser::ConvertToSRVSupportedFormat( d3dRaster->format );
    SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    SRVDesc.Texture2D.MipLevels = UINT32_MAX;

    if ( !CALL_D3D_API( dev->CreateShaderResourceView( m_pTextureResource, &SRVDesc, &m_pShaderView ),
                        "Failed to create shader resource view for 2D texture" ) )
        return;

}

CD3D1X2DTexture::~CD3D1X2DTexture()
{
    if ( m_pShaderView )
    {
        m_pShaderView->Release();
        m_pShaderView = nullptr;
    }
    if ( m_pLockedResource )
    {
        m_pLockedResource->Release();
        m_pLockedResource = nullptr;
    }
}

void CD3D1X2DTexture::SetDebugName( const std::string & name )
{
    CD3D1XBaseTexture::SetDebugName( name );
    if ( m_pShaderView )
        g_pDebug->SetD3DName( m_pShaderView, name + "(" + m_resourceTypeName + ", ShaderResourceView)" );
}

void CD3D1X2DTexture::Reallocate()
{
    if ( !m_bCanReallocate )
        return;
    if ( m_pShaderView )
    {
        m_pShaderView->Release();
        m_pShaderView = nullptr;
    }
    if ( m_pTextureResource )
    {
        m_pTextureResource->Release();
        m_pTextureResource = nullptr;
    }
    if ( m_pLockedResource )
    {
        m_pLockedResource->Release();
        m_pLockedResource = nullptr;
    }

    RwD3D1XRaster* d3dRaster = GetD3D1XRaster( m_pParent );
    ID3D11Device* dev = GET_D3D_DEVICE;

    // Base texture 2D creation
    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = m_pParent->width;
    desc.Height = m_pParent->height;
    desc.Format = CD3D1XEnumParser::ConvertToTextureBufferSupportedFormat( d3dRaster->format );
    desc.ArraySize = 1;
    desc.MipLevels = m_bHasMipMaps ? max( (int)log2( min( desc.Width, desc.Height ) ) - 2, 0 ) : 1;
    desc.SampleDesc.Quality = 0;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = ( m_bShaderAccess ? D3D11_BIND_SHADER_RESOURCE : 0 ) | m_dwBindFlags;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    if ( !CALL_D3D_API( dev->CreateTexture2D( &desc, NULL,
                                              reinterpret_cast<ID3D11Texture2D**>( &m_pTextureResource ) ),
                        "Failed to create 2D texture" ) )
        return;
    // If texture has no shader access(e.g. depth stencil surface for camera), than we don't need shader resource view
    if ( !m_bShaderAccess || m_pTextureResource == nullptr )
        return;

    D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};

    SRVDesc.Format = CD3D1XEnumParser::ConvertToSRVSupportedFormat( d3dRaster->format );
    SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    SRVDesc.Texture2D.MipLevels = UINT32_MAX;
    if ( !CALL_D3D_API( dev->CreateShaderResourceView( m_pTextureResource, &SRVDesc, &m_pShaderView ),
                        "Failed to create shader resource view for 2D texture" ) )
        return;
}

void * CD3D1X2DTexture::LockToRead()
{
    m_bIsLocked = true;

    RwD3D1XRaster* d3dRaster = GetD3D1XRaster( m_pParent );
    if ( m_pParent == nullptr || d3dRaster == nullptr )
    {
        g_pDebug->printError( "CD3D1X2DTexture::LockToRead() error: No raster exists" );
        return nullptr;
    }
    if ( m_pLockedResource == nullptr )
    {
        D3D11_TEXTURE2D_DESC desc{};
        desc.Width = m_pParent->width;
        desc.Height = m_pParent->height;
        desc.Format = d3dRaster->format;
        desc.MipLevels = m_bHasMipMaps ? max( (int)log2( min( desc.Width, desc.Height ) ) - 2, 0 ) : 1;
        desc.SampleDesc.Quality = 0;
        desc.SampleDesc.Count = 1;
        desc.ArraySize = 1;
        desc.Usage = D3D11_USAGE_STAGING;
        desc.BindFlags = 0;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;

        if ( !CALL_D3D_API( GET_D3D_DEVICE->CreateTexture2D( &desc, NULL, reinterpret_cast<ID3D11Texture2D**>( &m_pLockedResource ) ),
                            "Failed to create staging texture" ) )
            return nullptr;
        g_pDebug->SetD3DName( m_pLockedResource, "CD3D1X2DTexture::StagingTexture" );
    }
    // TODO: Add paletted texture support
    //if (!m_hasPalette) {
    m_lockedSubRes = {};
    auto context = GET_D3D_CONTEXT;
    if ( m_pLockedResource == nullptr )
        return nullptr;
    context->CopyResource( m_pLockedResource, m_pTextureResource );
    if ( !CALL_D3D_API( context->Map( m_pLockedResource, 0, D3D11_MAP_READ_WRITE, 0, &m_lockedSubRes ),
                        "Failed to map staging texture to sub-resource" ) )
    {
        m_bIsLocked = false;
        return nullptr;
    }
    m_pParent->stride = m_lockedSubRes.RowPitch;
    return m_lockedSubRes.pData;
    //}
    /*else {
    m_dataPtr = (BYTE*)malloc(sizeof(BYTE)*m_pParent->width*m_pParent->height);
    return m_dataPtr;
    }*/
}

void CD3D1X2DTexture::UnlockFromRead()
{
    auto context = GET_D3D_CONTEXT;
    m_bIsLocked = false;

    /*if (m_hasPalette) {
        context->CopyResource(m_pStagingTexture, m_pTexture);
        context->Map(m_pStagingTexture, 0, D3D11_MAP_READ_WRITE, 0, &m_mappedSubRes);
        UINT size = m_pParent->width*m_pParent->height;
        // convert raster data
        int* remappedPtr = (int*)m_mappedSubRes.pData;
        for (size_t i = 0; i < size; i++) {
            auto color = m_palette[m_dataPtr[i]];
            remappedPtr[i] = RWRGBALONG(color.red, color.green, color.blue, color.alpha);
        }
        free(m_dataPtr);
        m_dataPtr = nullptr;
    }*/
    context->Unmap( m_pLockedResource, 0 );
    context->CopyResource( m_pTextureResource, m_pLockedResource );

    if ( m_pLockedResource )
    {
        m_pLockedResource->Release();
        m_pLockedResource = nullptr;
    }
}
