// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "stdafx.h"
#include "CustomSeabedPipeline.h"
#include "D3DRenderer.h"
#include "CDebug.h"
#include "D3D1XShader.h"
#include "D3D1XStateManager.h"
#include "DeferredRenderer.h"
#include "D3D1XRenderBuffersManager.h"
#include "RwD3D1XEngine.h"

CCustomSeabedPipeline::CCustomSeabedPipeline() :
#ifndef DebuggingShaders
    CDeferredPipeline( "SACustomSeabed", GET_D3D_FEATURE_LVL >= D3D_FEATURE_LEVEL_11_0 )
#else
    CDeferredPipeline( L"SACustomSeabed", GET_D3D_FEATURE_LVL >= D3D_FEATURE_LEVEL_11_0 )
#endif // !DebuggingShaders
{
    ID3D11Device* pd3dDevice = GET_D3D_DEVICE;
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 12,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",		0, DXGI_FORMAT_R8G8B8A8_UNORM,		0, 24,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, 28,	D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    UINT numElements = ARRAYSIZE( layout );
    ID3DBlob* vsBlob = m_pVS->getBlob();
    // Create the input layout
    if ( FAILED( pd3dDevice->CreateInputLayout( layout, numElements, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_pVertexLayout ) ) )
        g_pDebug->printError( "Failed to create input layout" );


    D3D11_BUFFER_DESC bd;
    ZeroMemory( &bd, sizeof( bd ) );
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = 0x40000;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bd.MiscFlags = 0;
    bd.StructureByteStride = 0;

    if ( FAILED( pd3dDevice->CreateBuffer( &bd, nullptr, &m_pVertexBuffer ) ) )
        g_pDebug->printError( "Failed to create vertex buffer" );

    ZeroMemory( &bd, sizeof( bd ) );
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = 20000;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bd.MiscFlags = 0;
    bd.StructureByteStride = 0;

    if ( FAILED( pd3dDevice->CreateBuffer( &bd, nullptr, &m_pIndexBuffer ) ) )
        g_pDebug->printError( "Failed to create index buffer" );
    m_pSeaBedTexRaster = ( *(RwRaster**)0xC228B0 );
}

CCustomSeabedPipeline::~CCustomSeabedPipeline()
{
    if ( m_pIndexBuffer ) m_pIndexBuffer->Release();
    if ( m_pVertexBuffer ) m_pVertexBuffer->Release();
    if ( m_pVertexLayout ) m_pVertexLayout->Release();
}

void CCustomSeabedPipeline::RenderSeabed( RwIm3DVertex * verticles, UINT vertexCount, USHORT * indices, UINT indexCount )
{
    RwMatrix identity{};
    identity.right.x = 1.0f;
    identity.up.y = 1.0f;
    identity.at.z = 1.0f;
    identity.pad3 = 0x3F800000;
    g_pRenderBuffersMgr->UpdateWorldMatrix( &identity );
    auto devContext = GET_D3D_CONTEXT;
    g_pRenderBuffersMgr->SetMatrixBuffer();
    {
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        ZeroMemory( &mappedResource, sizeof( D3D11_MAPPED_SUBRESOURCE ) );
        //	Disable GPU access to the vertex buffer data.
        devContext->Map( m_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
        //	Update the vertex buffer here.
        memcpy( mappedResource.pData, verticles, sizeof( RwIm3DVertex )*vertexCount );
        //	Reenable GPU access to the vertex buffer data.
        devContext->Unmap( m_pVertexBuffer, 0 );

        ZeroMemory( &mappedResource, sizeof( D3D11_MAPPED_SUBRESOURCE ) );
        //	Disable GPU access to the index buffer data.
        devContext->Map( m_pIndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
        //	Update the index buffer here.
        memcpy( mappedResource.pData, &indices[0], sizeof( RwImVertexIndex )*indexCount );
        //	Reenable GPU access to the index buffer data.
        devContext->Unmap( m_pIndexBuffer, 0 );
    }

    g_pStateMgr->SetInputLayout( m_pVertexLayout );

    UINT stride = sizeof( RwIm3DVertex );
    UINT offset = 0;
    g_pStateMgr->SetVertexBuffer( m_pVertexBuffer, stride, offset );
    g_pStateMgr->SetIndexBuffer( m_pIndexBuffer );
    g_pStateMgr->SetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
    g_pStateMgr->SetRaster( m_pSeaBedTexRaster );
    if ( m_uiDeferredStage == 3 || m_uiDeferredStage == 4 )
    {
        m_pVoxelVS->Set();
        m_pVoxelGS->Set();
    }
    else
        m_pVS->Set();
    if ( m_uiDeferredStage == 1 )
        m_pDeferredPS->Set();
    else if ( m_uiDeferredStage == 2 )
        m_pShadowPS->Set();
    else if ( m_uiDeferredStage == 3 )
        m_pVoxelPS->Set();
    else if ( m_uiDeferredStage == 4 )
        m_pVoxelEmmissivePS->Set();
    else
        m_pPS->Set();

    g_pStateMgr->FlushStates();
    GET_D3D_RENDERER->DrawIndexed( indexCount, 0, 0 );
}
