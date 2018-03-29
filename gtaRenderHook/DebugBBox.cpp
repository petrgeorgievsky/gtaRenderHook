#include "stdafx.h"
#include "DebugBBox.h"
#include "D3D1XShader.h"
#include "RwD3D1XEngine.h"
#include "D3DRenderer.h"
#include "D3D1XStateManager.h"
#include "D3D1XRenderBuffersManager.h"
#include "DebugRendering.h"
// Verticles of unit box
RwV3d	DebugBBox::m_aVerticles[8] = 
{
	{  0.5, 0.5, 0.5 },
	{ -0.5, 0.5, 0.5 },
	{ -0.5,-0.5, 0.5 },
	{  0.5,-0.5, 0.5 },

	{  0.5, 0.5, -0.5 },
	{ -0.5, 0.5, -0.5 },
	{ -0.5,-0.5, -0.5 },
	{  0.5,-0.5, -0.5 },
};

USHORT	DebugBBox::m_aIndices[24] = {
	0,1,
	1,2,
	2,3,
	3,0,

	4,5,
	5,6,
	6,7,
	7,4,

	0,4,
	1,5,
	2,6,
	3,7
};
ID3D11InputLayout*      DebugBBox::m_pVertexLayout = nullptr;
ID3D11Buffer*           DebugBBox::m_pVertexBuffer = nullptr;
ID3D11Buffer*           DebugBBox::m_pIndexBuffer = nullptr;
CD3D1XShader*           DebugBBox::m_pVS = nullptr;
CD3D1XShader*           DebugBBox::m_pPS = nullptr;

DebugBBox::DebugBBox(RW::BBox bbox)
{
	auto pos = bbox.getCenter().getRWVector();
	m_WorldMatrix={};
	m_WorldMatrix.right.x = bbox.getSizeX();
	m_WorldMatrix.up.y = bbox.getSizeY();
	m_WorldMatrix.at.z = bbox.getSizeZ();
	m_WorldMatrix.pos = pos;
	m_WorldMatrix.pad3 = 0x3F800000;
}


DebugBBox::~DebugBBox()
{
}

void DebugBBox::Render()
{
	g_pRenderBuffersMgr->UpdateWorldMatrix(&m_WorldMatrix);

	g_pStateMgr->SetInputLayout(m_pVertexLayout);
	g_pStateMgr->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	g_pStateMgr->SetVertexBuffer(m_pVertexBuffer, sizeof(RwV3d), 0);
	g_pStateMgr->SetIndexBuffer(m_pIndexBuffer);
	g_pRenderBuffersMgr->SetMatrixBuffer();
	m_pVS->Set();
	m_pPS->Set();

	g_pStateMgr->FlushStates();
	GET_D3D_RENDERER->DrawIndexed(24, 0, 0);
}

void DebugBBox::Initialize()
{
	const auto shader_path = "shaders/debug.hlsl";

	m_pPS = new CD3D1XPixelShader(shader_path, "PS");
	m_pVS = new CD3D1XVertexShader(shader_path, "VS");

	ID3D11Device* pd3dDevice = GET_D3D_DEVICE;
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 0,	D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT numElements = ARRAYSIZE(layout);
	ID3DBlob* vsBlob = m_pVS->getBlob();
	// Create the input layout
	if (FAILED(pd3dDevice->CreateInputLayout(layout, numElements, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_pVertexLayout)))
		g_pDebug->printError("Failed to create input layout");


	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = 8*sizeof(RwV3d);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	bd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA initData{};
	initData.pSysMem = m_aVerticles;
	if (FAILED(pd3dDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer)))
		g_pDebug->printError("Failed to create vertex buffer");

	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = 24*sizeof(USHORT);
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	bd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA initDataIB{};
	initDataIB.pSysMem = m_aIndices;

	if (FAILED(pd3dDevice->CreateBuffer(&bd, &initDataIB, &m_pIndexBuffer)))
		g_pDebug->printError("Failed to create index buffer");
}

void DebugBBox::Shutdown()
{
	if (m_pVertexLayout) m_pVertexLayout->Release();
	if (m_pVertexBuffer) m_pVertexBuffer->Release();
	if (m_pIndexBuffer)  m_pIndexBuffer->Release();
	if (m_pPS) delete m_pPS;
	if (m_pVS) delete m_pVS;
}
