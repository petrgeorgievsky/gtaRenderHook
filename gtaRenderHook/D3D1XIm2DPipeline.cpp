#include "stdafx.h"
#include "D3D1XIm2DPipeline.h"
#include "CDebug.h"
#include "D3DRenderer.h"
#include "D3D1XShader.h"
#include "D3D1XStateManager.h"
#include "RwD3D1XEngine.h"
#include "D3D1XVertexDeclarationManager.h"

CD3D1XIm2DPipeline::CD3D1XIm2DPipeline():
#ifndef DebuggingShaders
	CD3D1XPipeline( "RwIm2D")
#else
	CD3D1XPipeline( L"RwIm2D")
#endif // !DebuggingShaders
{
	std::vector<D3D11_INPUT_ELEMENT_DESC> layout =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",		0, DXGI_FORMAT_R8G8B8A8_UNORM,		0, 16,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, 20,	D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	m_pVertexDecl = new CD3D1XVertexDeclaration(layout, sizeof(RwIm2DVertex), m_pVS);
	m_pVertexBuffer = new CD3D1XDynamicVertexBuffer(sizeof(RwIm2DVertex), 10000);
	m_pIndexBuffer = new CD3D1XDynamicIndexBuffer(10000);
}


CD3D1XIm2DPipeline::~CD3D1XIm2DPipeline()
{
	if (m_pIndexBuffer) delete m_pIndexBuffer;
	if (m_pVertexBuffer) delete m_pVertexBuffer;
	if (m_pVertexDecl) delete m_pVertexDecl;
}

void CD3D1XIm2DPipeline::Draw(RwPrimitiveType prim, RwIm2DVertex* verticles, RwUInt32 vertexCount)
{
	ID3D11DeviceContext* pImmediateContext = GET_D3D_CONTEXT;
	auto vertCount = vertexCount;
	// dynamic vertex buffer update
	if (prim != rwPRIMTYPETRIFAN) 
		m_pVertexBuffer->Update(&verticles[0], sizeof(RwIm2DVertex) * vertCount);
	else {
		std::vector<RwIm2DVertex> vertexArr;
		// d3d11 doesn't have triangle fan so we need to convert it to triangle list
		for (RwUInt32 i = 1; i < vertexCount-1; i++)
		{
			vertexArr.push_back(verticles[0]);
			vertexArr.push_back(verticles[i]);
			vertexArr.push_back(verticles[i+1]);
		}
		vertCount = vertexArr.size();
		m_pVertexBuffer->Update(vertexArr.data(), sizeof(RwIm2DVertex)* vertCount);
	}
	// initialize primitive info
	g_pStateMgr->SetInputLayout(m_pVertexDecl->getInputLayout());
	g_pStateMgr->SetVertexBuffer(m_pVertexBuffer->getBuffer(), sizeof(RwIm2DVertex), 0);
	g_pStateMgr->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_pVS->Set();
	m_pPS->Set();

	// draw primitive
	g_pStateMgr->FlushStates();
	pImmediateContext->Draw(vertCount, 0);
}

void CD3D1XIm2DPipeline::DrawIndexed(RwPrimitiveType prim, RwIm2DVertex *vertices, RwUInt32 numVertices, RwImVertexIndex *indices, RwUInt32 numIndices)
{
	// dynamic vertex and index buffer update
	if (prim != rwPRIMTYPETRIFAN) {
		m_pVertexBuffer->Update(&vertices[0], sizeof(RwIm2DVertex)*numVertices);
		m_pIndexBuffer->Update(&indices[0], sizeof(RwImVertexIndex)*numIndices);
	}
	else {
		g_pDebug->printError("Indexed triangle fan is not supported.");
		return;
	}

	// initialize primitive info
	g_pStateMgr->SetInputLayout(m_pVertexDecl->getInputLayout());
	g_pStateMgr->SetVertexBuffer(m_pVertexBuffer->getBuffer(), sizeof(RwIm2DVertex), 0);
	g_pStateMgr->SetIndexBuffer(m_pIndexBuffer->getBuffer());
	g_pStateMgr->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_pVS->Set();
	m_pPS->Set();
	// draw primitive
	g_pStateMgr->FlushStates();
	GET_D3D_RENDERER->DrawIndexed(numIndices, 0, 0);
}