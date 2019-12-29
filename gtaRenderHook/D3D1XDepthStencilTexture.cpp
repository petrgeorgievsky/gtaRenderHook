#include "stdafx.h"
#include "D3D1XDepthStencilTexture.h"
#include "CDebug.h"
// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "D3DSpecificHelpers.h"
#include "RwD3D1XEngine.h"
#include "D3D1XEnumParser.h"
#include "D3DRenderer.h"

CD3D1XDepthStencilTexture::CD3D1XDepthStencilTexture( RwRaster * parent )
    : CD3D1X2DTexture( parent, D3D11_BIND_DEPTH_STENCIL, "DepthStencilTexture", false, true, true )
{
    RwD3D1XRaster* d3dRaster = GetD3D1XRaster( m_pParent );
    ID3D11Device* dev = GET_D3D_DEVICE;

    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV{};
    descDSV.Format = d3dRaster->format;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;

    if ( !CALL_D3D_API( dev->CreateDepthStencilView( m_pTextureResource, &descDSV, &m_pDepthStencilView ),
                        "Failed to create depth stencil view" ) )
        return;
}

CD3D1XDepthStencilTexture::~CD3D1XDepthStencilTexture()
{
    if ( m_pDepthStencilView )
    {
        m_pDepthStencilView->Release();
        m_pDepthStencilView = nullptr;
    }
}

void CD3D1XDepthStencilTexture::SetDebugName( const std::string & name )
{
    CD3D1X2DTexture::SetDebugName( name );
    if ( m_pDepthStencilView )
        g_pDebug->SetD3DName( m_pDepthStencilView, name + "(" + m_resourceTypeName + ", DepthStencilView)" );
}

void CD3D1XDepthStencilTexture::Reallocate()
{
    // Release allocated resources
    if ( m_pDepthStencilView )
    {
        m_pDepthStencilView->Release();
        m_pDepthStencilView = nullptr;
    }
    // Realloc resources
    CD3D1X2DTexture::Reallocate();

    // Recreate views
    RwD3D1XRaster* d3dRaster = GetD3D1XRaster( m_pParent );
    ID3D11Device* dev = GET_D3D_DEVICE;

    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV{};
    descDSV.Format = d3dRaster->format;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;

    if ( !CALL_D3D_API( dev->CreateDepthStencilView( m_pTextureResource, &descDSV, &m_pDepthStencilView ),
                        "Failed to create depth stencil view" ) )
        return;
}
