// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
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
CD3D1XVertexDeclaration*	DebugBBox::m_pVertexDecl = nullptr;
CD3D1XVertexBuffer*			DebugBBox::m_pVertexBuffer = nullptr;
CD3D1XIndexBuffer*			DebugBBox::m_pIndexBuffer = nullptr;
CD3D1XShader*				DebugBBox::m_pVS = nullptr;
CD3D1XShader*				DebugBBox::m_pPS = nullptr;

DebugBBox::DebugBBox(RW::BBox bbox)
{
	auto pos = bbox.getCenter().getRWVector();
	m_WorldMatrix={};
	m_WorldMatrix.right.x	= bbox.getSizeX();
	m_WorldMatrix.up.y		= bbox.getSizeY();
	m_WorldMatrix.at.z		= bbox.getSizeZ();
	m_WorldMatrix.pos		= pos;
	m_WorldMatrix.pad3		= 0x3F800000;
}

DebugBBox::DebugBBox(RW::BBox bbox, RW::Matrix rotationMatrix)
{
	auto pos = bbox.getCenter();
	RW::Matrix wsMatrix = { RW::V4d{ bbox.getSizeX(),0.0,0.0,0.0 }, RW::V4d{ 0.0,bbox.getSizeY(),0.0,0.0 },
							RW::V4d{ 0.0,0.0,bbox.getSizeZ(),0.0 }, RW::V4d{ pos, 1.0 } };
	m_WorldMatrix = (wsMatrix*rotationMatrix).getRWMatrix();
}


DebugBBox::~DebugBBox()
{
}

void DebugBBox::Render()
{
	// TODO: replace all API-specific calls to engine-specific.
	// Set mesh info
	g_pStateMgr->SetInputLayout(m_pVertexDecl->getInputLayout());
	g_pStateMgr->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	g_pStateMgr->SetVertexBuffer(m_pVertexBuffer->getBuffer(), sizeof(RwV3d), 0);
	g_pStateMgr->SetIndexBuffer(m_pIndexBuffer->getBuffer());

	// Set transformation info
	g_pRenderBuffersMgr->UpdateWorldMatrix(&m_WorldMatrix);
	g_pRenderBuffersMgr->SetMatrixBuffer();

	// Set shaders
	m_pVS->Set();
	m_pPS->Set();

	// Flush states and draw on screen
	g_pStateMgr->FlushStates();
	GET_D3D_RENDERER->DrawIndexed(24, 0, 0);
}

void DebugBBox::Initialize()
{
	const auto shader_path = "shaders/debug.hlsl";

	m_pPS = new CD3D1XPixelShader(shader_path, "PS");
	m_pVS = new CD3D1XVertexShader(shader_path, "VS");

	std::vector<D3D11_INPUT_ELEMENT_DESC> layout =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 0,	D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	m_pVertexDecl = new CD3D1XVertexDeclaration(layout, 12, m_pVS);
	
	D3D11_SUBRESOURCE_DATA initData{};
	initData.pSysMem = m_aVerticles;
	m_pVertexBuffer = new CD3D1XVertexBuffer(8 * sizeof(RwV3d), &initData);

	initData={};
	initData.pSysMem = m_aIndices;
	m_pIndexBuffer = new CD3D1XIndexBuffer(24, &initData);
}

void DebugBBox::Shutdown()
{
	delete m_pVertexDecl;
	delete m_pVertexBuffer;
	delete m_pIndexBuffer;
	delete m_pPS;
	delete m_pVS;
}
