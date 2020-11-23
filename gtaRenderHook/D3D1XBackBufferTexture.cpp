// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "stdafx.h"
#include "D3D1XBackBufferTexture.h"
#include "RwD3D1XEngine.h"
#include "D3DRenderer.h"
#include "D3DSpecificHelpers.h"

CD3D1XBackBufferTexture::CD3D1XBackBufferTexture( RwRaster* parent ) :
    CD3D1XBaseTexture( parent, eTextureDimension::TT_2D, "BackBufferTexture" )
{
    if ( !CALL_D3D_API( GET_D3D_SWAP_CHAIN->GetBuffer( 0, __uuidof( ID3D11Texture2D ), reinterpret_cast<void**>( &m_pTextureResource ) ), "Failed to get back buffer texture." ) )
        return;
    if ( m_pTextureResource == nullptr )
        return;
    if ( !CALL_D3D_API( GET_D3D_DEVICE->CreateRenderTargetView( m_pTextureResource, nullptr, &m_pRenderTargetView ), "Failed to create render target view." ) )
        return;

    m_pTextureResource->Release();
    m_pTextureResource = nullptr;
}


CD3D1XBackBufferTexture::~CD3D1XBackBufferTexture()
{
    if ( m_pRenderTargetView )
    {
        m_pRenderTargetView->Release();
        m_pRenderTargetView = nullptr;
    }
}

void CD3D1XBackBufferTexture::SetDebugName( const std::string & name )
{
    CD3D1XBaseTexture::SetDebugName( name );
    if ( m_pRenderTargetView )
        g_pDebug->SetD3DName( m_pRenderTargetView, name + "(" + m_resourceTypeName + ", RenderTargetView)" );
}
