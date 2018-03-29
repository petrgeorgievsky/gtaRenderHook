#include "stdafx.h"
#include "D3D1XIm3DPipeline.h"
#include "CDebug.h"
#include "D3DRenderer.h"
#include "D3D1XShader.h"
#include "D3D1XStateManager.h"
#include "RwD3D1XEngine.h"
 
CD3D1XIm3DPipeline::CD3D1XIm3DPipeline():
#ifndef DebuggingShaders
	CD3D1XPipeline("RwIm3D")
#else
	CD3D1XPipeline(L"RwIm3D")
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
	UINT numElements = ARRAYSIZE(layout);
	ID3DBlob* vsBlob = m_pVS->getBlob();
	// Create the input layout
	if (FAILED(pd3dDevice->CreateInputLayout(layout, numElements, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_pVertexLayout)))
		g_pDebug->printError("Failed to create input layout");
	

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = 0x40000;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bd.MiscFlags = 0;
	bd.StructureByteStride = 0;

	if (FAILED(pd3dDevice->CreateBuffer(&bd, nullptr, &m_pVertexBuffer)))
		g_pDebug->printError("Failed to create vertex buffer");

	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = 20000;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bd.MiscFlags = 0;
	bd.StructureByteStride = 0;

	if (FAILED(pd3dDevice->CreateBuffer(&bd, nullptr, &m_pIndexBuffer)))
		g_pDebug->printError("Failed to create index buffer");

}


CD3D1XIm3DPipeline::~CD3D1XIm3DPipeline()
{
	if (m_pIndexBuffer) m_pIndexBuffer->Release();
	if (m_pVertexBuffer) m_pVertexBuffer->Release();
	if (m_pVertexLayout) m_pVertexLayout->Release();
}

RwBool CD3D1XIm3DPipeline::SubmitNode()
{
	ID3D11DeviceContext* pImmediateContext = GET_D3D_CONTEXT;
	rwIm3DPool* pool = rwD3D9ImmPool;

	if (pool->stash.primType!=rwPRIMTYPETRIFAN) 
	{
		if (pool->stash.indices&&pool->stash.numIndices>0) {
			{
				D3D11_MAPPED_SUBRESOURCE mappedResource;
				ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
				//	Disable GPU access to the vertex buffer data.
				pImmediateContext->Map(m_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
				//	Update the vertex buffer here.
				memcpy(mappedResource.pData, rwD3D9ImmPool->elements, sizeof(RwIm3DVertex)*rwD3D9ImmPool->numElements);
				//	Reenable GPU access to the vertex buffer data.
				pImmediateContext->Unmap(m_pVertexBuffer, 0);

				ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
				//	Disable GPU access to the index buffer data.
				pImmediateContext->Map(m_pIndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
				//	Update the index buffer here.
				memcpy(mappedResource.pData, &rwD3D9ImmPool->stash.indices[0], sizeof(RwImVertexIndex)*size_t(rwD3D9ImmPool->stash.numIndices));
				//	Reenable GPU access to the index buffer data.
				pImmediateContext->Unmap(m_pIndexBuffer, 0);
			}

			g_pStateMgr->SetInputLayout(m_pVertexLayout);

			UINT stride = sizeof(RwIm3DVertex);
			UINT offset = 0;
			g_pStateMgr->SetVertexBuffer(m_pVertexBuffer, stride, offset);
			g_pStateMgr->SetIndexBuffer(m_pIndexBuffer);
			if (rwD3D9ImmPool->stash.primType == rwPRIMTYPETRILIST)
				g_pStateMgr->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			else if (rwD3D9ImmPool->stash.primType == rwPRIMTYPETRISTRIP)
				g_pStateMgr->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			else if (rwD3D9ImmPool->stash.primType == rwPRIMTYPELINELIST)
				g_pStateMgr->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
			else
				g_pDebug->printMsg("D3D1XIm3DPipeline: Unknown primitive type:"+to_string(rwD3D9ImmPool->stash.primType)+", could be poly line or point list.",2);
			m_pVS->Set();
			m_pPS->Set();

			g_pStateMgr->FlushStates();
			pImmediateContext->DrawIndexed(pool->stash.numIndices, 0, 0);
		}
		else {
			{
				D3D11_MAPPED_SUBRESOURCE mappedResource;
				ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
				//	Disable GPU access to the vertex buffer data.
				pImmediateContext->Map(m_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
				//	Update the vertex buffer here.
				memcpy(mappedResource.pData, rwD3D9ImmPool->elements, sizeof(RwIm3DVertex)*rwD3D9ImmPool->numElements);
				//	Reenable GPU access to the vertex buffer data.
				pImmediateContext->Unmap(m_pVertexBuffer, 0);
			}

			g_pStateMgr->SetInputLayout(m_pVertexLayout);

			UINT stride = sizeof(RwIm3DVertex);
			UINT offset = 0;
			g_pStateMgr->SetVertexBuffer(m_pVertexBuffer, stride, offset);

			if (rwD3D9ImmPool->stash.primType == rwPRIMTYPETRILIST)
				g_pStateMgr->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			else if (rwD3D9ImmPool->stash.primType == rwPRIMTYPETRISTRIP)
				g_pStateMgr->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			else if (rwD3D9ImmPool->stash.primType == rwPRIMTYPELINELIST)
				g_pStateMgr->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
			else
				g_pDebug->printMsg("D3D1XIm3DPipeline: Unknown primitive type:" + to_string(rwD3D9ImmPool->stash.primType) + ", could be poly line or point list.", 2);
			m_pVS->Set();
			m_pPS->Set();

			g_pStateMgr->FlushStates();
			pImmediateContext->Draw(rwD3D9ImmPool->numElements, 0);
		}
	}
	else
	{
		g_pDebug->printMsg("D3D1XIm3DPipeline: Triangle fan primitive is currently unsuported.", 2);
	}
	return true;
}
