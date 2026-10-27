// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "stdafx.h"
#include "D3D1XIm3DPipeline.h"
#include "CDebug.h"
#include "D3DRenderer.h"
#include "D3D1XShader.h"
#include "D3D1XStateManager.h"
#include "RwD3D1XEngine.h"
#include "D3D1XEnumParser.h"

CD3D1XIm3DPipeline::CD3D1XIm3DPipeline() :
#ifndef DebuggingShaders
    CD3D1XPipeline( "RwIm3D" )
#else
    CD3D1XPipeline( L"RwIm3D" )
#endif // !DebuggingShaders
{
    std::vector<D3D11_INPUT_ELEMENT_DESC> layout =
    {
        { "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 12,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",		0, DXGI_FORMAT_R8G8B8A8_UNORM,		0, 24,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, 28,	D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    m_pVertexDeclaration = new CD3D1XVertexDeclaration( layout, sizeof( RwIm3DVertex ), m_pVS );
    m_pVertexBuffer = new CD3D1XDynamicVertexBuffer( sizeof( RwIm3DVertex ), m_uiVertexCapacity );
    m_pIndexBuffer = new CD3D1XDynamicIndexBuffer( m_uiIndexCapacity );
    m_pVertexBuffer->SetDebugName( "Im3D_DynamicVB" );
    m_pIndexBuffer->SetDebugName( "Im3D_DynamicIB" );
}

// RenderWare takes up to 65535 vertices and indices per Im3D batch. Plugins draw more than the initial 10000 (e.g. the
// LOD lights of Project2DFX), so the buffers grow to fit a batch instead of being written past their end.
void CD3D1XIm3DPipeline::EnsureCapacity( unsigned int vertexCount, unsigned int indexCount )
{
    if ( vertexCount > m_uiVertexCapacity )
    {
        m_uiVertexCapacity = max( vertexCount, m_uiVertexCapacity * 2 );
        delete m_pVertexBuffer;
        m_pVertexBuffer = new CD3D1XDynamicVertexBuffer( sizeof( RwIm3DVertex ), m_uiVertexCapacity );
        m_pVertexBuffer->SetDebugName( "Im3D_DynamicVB" );
    }
    if ( indexCount > m_uiIndexCapacity )
    {
        m_uiIndexCapacity = max( indexCount, m_uiIndexCapacity * 2 );
        delete m_pIndexBuffer;
        m_pIndexBuffer = new CD3D1XDynamicIndexBuffer( m_uiIndexCapacity );
        m_pIndexBuffer->SetDebugName( "Im3D_DynamicIB" );
    }
}


CD3D1XIm3DPipeline::~CD3D1XIm3DPipeline()
{
    delete m_pVertexDeclaration;
    delete m_pIndexBuffer;
    delete m_pVertexBuffer;
}

RwBool CD3D1XIm3DPipeline::SubmitNode()
{
    ID3D11DeviceContext* pImmediateContext = GET_D3D_CONTEXT;
    //rwIm3DPool* pool = rwD3D9ImmPool;
    auto im3dPoolStash = rwD3D9ImmPool->stash;

    if ( im3dPoolStash.primType != rwPRIMTYPETRIFAN )
    {
        EnsureCapacity( rwD3D9ImmPool->numElements, im3dPoolStash.indices ? im3dPoolStash.numIndices : 0 );
        // update verticles
        m_pVertexBuffer->Update( &( (RwIm3DVertex*)rwD3D9ImmPool->elements )[0], sizeof( RwIm3DVertex )*rwD3D9ImmPool->numElements );
        // initialize vertex declaration and buffer
        g_pStateMgr->SetInputLayout( m_pVertexDeclaration->getInputLayout() );
        g_pStateMgr->SetVertexBuffer( m_pVertexBuffer->getBuffer(), sizeof( RwIm3DVertex ), 0 );
        // initialize primitive topology
        g_pStateMgr->SetPrimitiveTopology( CD3D1XEnumParser::ConvertPrimTopology( im3dPoolStash.primType ) );
        // initialize shaders
        m_pVS->Set();
        m_pPS->Set();
        // draw mesh
        if ( im3dPoolStash.indices && im3dPoolStash.numIndices > 0 )
        {
            // update index buffer
            m_pIndexBuffer->Update( &im3dPoolStash.indices[0], sizeof( RwImVertexIndex )*im3dPoolStash.numIndices );
            // initialize vertex buffer
            g_pStateMgr->SetIndexBuffer( m_pIndexBuffer->getBuffer() );

            g_pStateMgr->FlushStates();
            pImmediateContext->DrawIndexed( im3dPoolStash.numIndices, 0, 0 );
        }
        else
        {
            g_pStateMgr->FlushStates();
            pImmediateContext->Draw( rwD3D9ImmPool->numElements, 0 );
        }
    }
    else
    {
        // TODO: perhaps add triangle fan rendering
        g_pDebug->printMsg( "D3D1XIm3DPipeline: Triangle fan primitive is currently unsuported.", 0 );
    }
    return true;
}
