#include "stdafx.h"
#include "FullscreenQuad.h"
#include "RwRenderEngine.h"
#include "RwD3D1XEngine.h"
#include "D3DRenderer.h"
#include "D3D1XShader.h"
#include "D3D1XStateManager.h"

ID3D11Buffer* CFullscreenQuad::m_quadVB = nullptr;
ID3D11Buffer* CFullscreenQuad::m_quadIB = nullptr;
CD3D1XShader* CFullscreenQuad::m_quadVS = nullptr;
ID3D11InputLayout* CFullscreenQuad::m_quadIL = nullptr;

void CFullscreenQuad::Init()
{
	ID3D11Device* device = GET_D3D_DEVICE;
	m_quadVS = new CD3D1XVertexShader("shaders/Quad.hlsl", "VS");
	
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	device->CreateInputLayout(layout, 2, m_quadVS->getBlob()->GetBufferPointer(), m_quadVS->getBlob()->GetBufferSize(), &m_quadIL);

	// Create and initialize the vertex and index buffers
	QuadVertex verts[4] =
	{
		{ RwV4d{ 1, 1, 1, 1 }, RwV2d{ 1, 0 } },
		{ RwV4d{ 1, -1, 1, 1 }, RwV2d{ 1, 1 } },
		{ RwV4d{ -1, -1, 1, 1 }, RwV2d{ 0, 1 } },
		{ RwV4d{ -1, 1, 1, 1 }, RwV2d{ 0, 0 } }
	};

	D3D11_BUFFER_DESC desc;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.ByteWidth = sizeof(verts);
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = verts;
	initData.SysMemPitch = 0;
	initData.SysMemSlicePitch = 0;
	device->CreateBuffer(&desc, &initData, &m_quadVB);

	unsigned short indices[6] = { 0, 1, 2, 2, 3, 0 };

	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.ByteWidth = sizeof(indices);
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	desc.CPUAccessFlags = 0;
	initData.pSysMem = indices;
	device->CreateBuffer(&desc, &initData, &m_quadIB);
}

void CFullscreenQuad::Shutdown()
{
	if (m_quadVB) m_quadVB->Release();
	if (m_quadIB) m_quadIB->Release();
	if (m_quadIL) m_quadIL->Release();
	delete m_quadVS;
}

void CFullscreenQuad::Draw()
{
	ID3D11DeviceContext* context = GET_D3D_CONTEXT;
	//g_pDebug->printMsg("Fullscreen quad render start.");
	g_pStateMgr->SetInputLayout(m_quadIL);

	// Set the vertex buffer
	UINT stride = sizeof(QuadVertex);
	UINT offset = 0;
	ID3D11Buffer* vertexBuffers[1] = { m_quadVB };
	//g_pDebug->printMsg("VertexBuffer seting.");
	g_pStateMgr->SetVertexBuffer(vertexBuffers[0], stride, offset);

	// Set the index buffer
	//g_pDebug->printMsg("IndexBuffer seting.");
	g_pStateMgr->SetIndexBuffer(m_quadIB);

	// Set primitive topology
	//g_pDebug->printMsg("Topology seting.");
	g_pStateMgr->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Set shader
	//g_pDebug->printMsg("Shader seting.");
	m_quadVS->Set();

	// Flush states
	//g_pDebug->printMsg("States flush.");
	g_pStateMgr->FlushStates();

	// Draw quad
	context->DrawIndexed(6, 0, 0);
	//g_pDebug->printMsg("Fullscreen quad render end.");
}
