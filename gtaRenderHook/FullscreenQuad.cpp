// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "stdafx.h"
#include "FullscreenQuad.h"
#include "RwRenderEngine.h"
#include "RwD3D1XEngine.h"
#include "D3DRenderer.h"
#include "D3D1XShader.h"
#include "D3D1XVertexDeclaration.h"
#include "D3D1XStateManager.h"
#include "D3D1XVertexBuffer.h"
#include "D3D1XIndexBuffer.h"

CD3D1XVertexBuffer* CFullscreenQuad::m_quadVB = nullptr;
CD3D1XIndexBuffer* CFullscreenQuad::m_quadIB = nullptr;
CD3D1XShader* CFullscreenQuad::m_quadVS = nullptr;
CD3D1XShader* CFullscreenQuad::m_BlitPS = nullptr;
CD3D1XVertexDeclaration* CFullscreenQuad::m_pVertexDecl = nullptr;
RwRaster*	CFullscreenQuad::m_pBlitRaster = nullptr;

void CFullscreenQuad::Init()
{
    m_quadVS = new CD3D1XVertexShader( "shaders/Quad.hlsl", "VS" );
    m_BlitPS = new CD3D1XPixelShader( "shaders/Quad.hlsl", "BlitPS" );

    std::vector<D3D11_INPUT_ELEMENT_DESC> layout =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    m_pVertexDecl = new CD3D1XVertexDeclaration( layout, 6, m_quadVS );

    // Create and initialize the vertex and index buffers
    QuadVertex verts[4] =
    {
        { RwV4d{ 1, 1, 1, 1 }, RwV2d{ 1, 0 } },
        { RwV4d{ 1, -1, 1, 1 }, RwV2d{ 1, 1 } },
        { RwV4d{ -1, -1, 1, 1 }, RwV2d{ 0, 1 } },
        { RwV4d{ -1, 1, 1, 1 }, RwV2d{ 0, 0 } }
    };
    unsigned short indices[6] = { 0, 1, 2, 2, 3, 0 };

    D3D11_SUBRESOURCE_DATA initData{};

    initData.pSysMem = verts;
    m_quadVB = new CD3D1XVertexBuffer( 4 * sizeof( QuadVertex ), &initData );

    initData.pSysMem = indices;
    m_quadIB = new CD3D1XIndexBuffer( 6, &initData );
    m_pBlitRaster = RwRasterCreate( RsGlobal.maximumWidth, RsGlobal.maximumHeight, 32, rwRASTERTYPECAMERATEXTURE | rwRASTERFORMAT1555 );

}

void CFullscreenQuad::Shutdown()
{
    delete m_BlitPS;
    delete m_quadVB;
    delete m_quadIB;
    delete m_pVertexDecl;
    delete m_quadVS;
    if ( m_pBlitRaster ) RwRasterDestroy( m_pBlitRaster );
}

void CFullscreenQuad::Draw()
{
    g_pStateMgr->SetInputLayout( m_pVertexDecl->getInputLayout() );

    // Set the vertex buffer
    g_pStateMgr->SetVertexBuffer( m_quadVB->getBuffer(), sizeof( QuadVertex ), 0 );

    // Set the index buffer
    g_pStateMgr->SetIndexBuffer( m_quadIB->getBuffer() );

    // Set primitive topology
    g_pStateMgr->SetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    // Set shader
    m_quadVS->Set();

    // Flush states
    g_pStateMgr->FlushStates();

    // Draw quad
    GET_D3D_RENDERER->DrawIndexed( 6, 0, 0 );
}

void CFullscreenQuad::Copy( RwRaster * from, RwRaster* zBuffer )
{
    g_pRwCustomEngine->SetRenderTargets( &m_pBlitRaster, nullptr, 1 );
    g_pStateMgr->FlushRenderTargets();
    g_pStateMgr->SetRaster( from );
    m_BlitPS->Set();
    Draw();
}

void CFullscreenQuad::Copy( RwRaster * from, RwRaster * zBuffer, RwRaster * to )
{
    g_pRwCustomEngine->SetRenderTargets( &to, zBuffer, 1 );
    g_pStateMgr->FlushRenderTargets();
    g_pStateMgr->SetRaster( from );
    m_BlitPS->Set();
    Draw();
}

void CFullscreenQuad::QueueTextureReload()
{
    CRwD3D1XEngine* dxEngine = (CRwD3D1XEngine*)g_pRwCustomEngine;
    if ( dxEngine->m_bScreenSizeChanged )
    {
        m_pBlitRaster->width = RsGlobal.maximumWidth;
        m_pBlitRaster->height = RsGlobal.maximumHeight;
        dxEngine->m_pRastersToReload.push_back( m_pBlitRaster );
    }
}
