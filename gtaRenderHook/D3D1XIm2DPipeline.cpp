#include "stdafx.h"
#include "D3D1XIm2DPipeline.h"
#include "CDebug.h"
#include "D3DRenderer.h"
#include "D3D1XShader.h"
#include "D3D1XStateManager.h"
#include "RwD3D1XEngine.h"

CD3D1XIm2DPipeline::CD3D1XIm2DPipeline():
#ifndef DebuggingShaders
	CD3D1XPipeline( "RwIm2D")
#else
	CD3D1XPipeline( L"RwIm2D")
#endif // !DebuggingShaders
{
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",		0, DXGI_FORMAT_R8G8B8A8_UNORM,		0, 16,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, 20,	D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT numElements = ARRAYSIZE(layout);

	ID3DBlob* vsBlob = m_pVS->getBlob();
	ID3D11Device* pd3dDevice = GET_D3D_DEVICE;
	// Create the input layout
	if (FAILED(pd3dDevice->CreateInputLayout(layout, numElements, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_pVertexLayout)))
		g_pDebug->printError("Failed to create input layout");

	D3D11_BUFFER_DESC bd {};
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = 0x40000;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bd.MiscFlags = 0;
	bd.StructureByteStride = 0;

	if (FAILED(pd3dDevice->CreateBuffer(&bd, nullptr, &m_pVertexBuffer)))
		g_pDebug->printError("Failed to create vertex buffer");
	bd = {};
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = 20000;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bd.MiscFlags = 0;
	bd.StructureByteStride = 0;

	if (FAILED(pd3dDevice->CreateBuffer(&bd, nullptr, &m_pIndexBuffer)))
		g_pDebug->printError("Failed to create index buffer");
}


CD3D1XIm2DPipeline::~CD3D1XIm2DPipeline()
{
	if (m_pIndexBuffer) m_pIndexBuffer->Release();
	if (m_pVertexBuffer) m_pVertexBuffer->Release();
	if (m_pVertexLayout) m_pVertexLayout->Release();
}

void CD3D1XIm2DPipeline::Draw( RwPrimitiveType prim, RwIm2DVertex* verticles, RwUInt32 vertexCount)
{
	ID3D11DeviceContext* pImmediateContext = GET_D3D_CONTEXT;
	size_t vertCount = static_cast<size_t>(vertexCount);
	if (prim != rwPRIMTYPETRIFAN) {
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
		//	Disable GPU access to the vertex buffer data.
		pImmediateContext->Map(m_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		//	Update the vertex buffer here.
		memcpy(mappedResource.pData, &verticles[0], sizeof(RwIm2DVertex)*vertCount);
		//	Reenable GPU access to the vertex buffer data.
		pImmediateContext->Unmap(m_pVertexBuffer, 0);
	}
	else {
		std::vector<RwIm2DVertex> vertexArr;
		for (size_t i = 1; i < static_cast<size_t>(vertexCount)-1; i++)
		{
			vertexArr.push_back(verticles[0]);
			vertexArr.push_back(verticles[i]);
			vertexArr.push_back(verticles[i+1]);
		}
		vertCount = vertexArr.size();
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
		//	Disable GPU access to the vertex buffer data.
		pImmediateContext->Map(m_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		//	Update the vertex buffer here.
		memcpy(mappedResource.pData, vertexArr.data(), sizeof(RwIm2DVertex)*vertCount);
		//	Reenable GPU access to the vertex buffer data.
		pImmediateContext->Unmap(m_pVertexBuffer, 0);
	}

	g_pStateMgr->SetInputLayout(m_pVertexLayout);

	UINT stride = sizeof(RwIm2DVertex);
	UINT offset = 0;
	g_pStateMgr->SetVertexBuffer( m_pVertexBuffer, stride, offset);
	g_pStateMgr->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_pVS->Set();
	m_pPS->Set();

	g_pStateMgr->FlushStates();
	pImmediateContext->Draw(static_cast<UINT>(vertCount), 0);
}

void CD3D1XIm2DPipeline::DrawIndexed(RwPrimitiveType prim, RwIm2DVertex *vertices, RwUInt32 numVertices, RwImVertexIndex *indices, RwUInt32 numIndices)
{
	ID3D11DeviceContext* pImmediateContext = GET_D3D_CONTEXT;
	size_t vertCount = static_cast<size_t>(numVertices);
	if (prim != rwPRIMTYPETRIFAN) {
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
		//	Disable GPU access to the vertex buffer data.
		pImmediateContext->Map(m_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		//	Update the vertex buffer here.
		memcpy(mappedResource.pData, &vertices[0], sizeof(RwIm2DVertex)*vertCount);
		//	Reenable GPU access to the vertex buffer data.
		pImmediateContext->Unmap(m_pVertexBuffer, 0);

		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
		//	Disable GPU access to the index buffer data.
		pImmediateContext->Map(m_pIndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		//	Update the index buffer here.
		memcpy(mappedResource.pData, &indices[0], sizeof(RwImVertexIndex)*size_t(numIndices));
		//	Reenable GPU access to the index buffer data.
		pImmediateContext->Unmap(m_pIndexBuffer, 0);
	}
	else {
		g_pDebug->printError("unimplemented code");
	}

	g_pStateMgr->SetInputLayout(m_pVertexLayout);

	UINT stride = sizeof(RwIm2DVertex);
	UINT offset = 0;
	g_pStateMgr->SetVertexBuffer(m_pVertexBuffer, stride, offset);
	g_pStateMgr->SetIndexBuffer(m_pIndexBuffer);
	g_pStateMgr->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_pVS->Set();
	m_pPS->Set();

	g_pStateMgr->FlushStates();
	GET_D3D_RENDERER->DrawIndexed(numIndices, 0, 0);
}