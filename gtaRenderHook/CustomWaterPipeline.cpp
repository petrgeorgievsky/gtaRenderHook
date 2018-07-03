// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "stdafx.h"
#include "CustomWaterPipeline.h"
#include "D3DRenderer.h"
#include "CDebug.h"
#include "D3D1XShader.h"
#include "D3D1XStateManager.h"
#include "D3D1XRenderBuffersManager.h"
#include "RwD3D1XEngine.h"
WaterSettingsBlock gWaterSettings;
CCustomWaterPipeline::CCustomWaterPipeline() :
#ifndef DebuggingShaders
	CD3D1XPipeline( "SACustomWater")
#else
	CD3D1XPipeline( L"SACustomWater")
#endif // !DebuggingShaders
{
	gWaterSettings.m_aShaderPointers.push_back(m_pPS);
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
	m_pDS = new CD3D1XDomainShader("shaders/SACustomWater.hlsl", "DS");
	m_pHS = new CD3D1XHullShader("shaders/SACustomWater.hlsl", "HS");
}


CCustomWaterPipeline::~CCustomWaterPipeline()
{
	delete m_pDS;
	delete m_pHS;	
	if (m_pIndexBuffer) m_pIndexBuffer->Release();
	if (m_pVertexBuffer) m_pVertexBuffer->Release();
	if (m_pVertexLayout) m_pVertexLayout->Release();
}

void CCustomWaterPipeline::RenderWater(RwIm3DVertex * verticles, UINT vertexCount, USHORT * indices, UINT indexCount)
{
	m_pWaterRaster = (*(RwRaster**)0xC228A8);
	m_pWaterWakeRaster = (*(RwRaster**)0xC228B8);
	RwMatrix identity{};
	identity.right.x = 1.0f;
	identity.up.y = 1.0f;
	identity.at.z = 1.0f;
	identity.pad3 = 0x3F800000;
	g_pRenderBuffersMgr->UpdateWorldMatrix(&identity);
	auto devContext = GET_D3D_CONTEXT;
	g_pRenderBuffersMgr->SetMatrixBuffer();
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
		//	Disable GPU access to the vertex buffer data.
		devContext->Map(m_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		//	Update the vertex buffer here.
		memcpy(mappedResource.pData, verticles, sizeof(RwIm3DVertex)*vertexCount);
		//	Reenable GPU access to the vertex buffer data.
		devContext->Unmap(m_pVertexBuffer, 0);

		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
		//	Disable GPU access to the index buffer data.
		devContext->Map(m_pIndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		//	Update the index buffer here.
		memcpy(mappedResource.pData, &indices[0], sizeof(RwImVertexIndex)*indexCount);
		//	Reenable GPU access to the index buffer data.
		devContext->Unmap(m_pIndexBuffer, 0);
	}

	g_pStateMgr->SetInputLayout(m_pVertexLayout);

	UINT stride = sizeof(RwIm3DVertex);
	UINT offset = 0;
	g_pStateMgr->SetVertexBuffer(m_pVertexBuffer, stride, offset);
	g_pStateMgr->SetIndexBuffer(m_pIndexBuffer);
	g_pStateMgr->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	g_pStateMgr->SetRaster(m_pWaterRaster);
	g_pStateMgr->SetRaster(m_pWaterWakeRaster, 5);
	//g_pStateMgr->SetFillMode(D3D11_FILL_WIREFRAME);
	m_pVS->Set();
	m_pPS->Set();
	m_pDS->Set();
	m_pHS->Set(); 
	g_pStateMgr->FlushStates();
	GET_D3D_RENDERER->DrawIndexed(indexCount, 0, 0);
	m_pDS->ReSet();
	m_pHS->ReSet();
	//g_pStateMgr->SetFillMode(D3D11_FILL_SOLID);
}
void TW_CALL ReloadWaterShadersCallBack(void *value)
{
	gWaterSettings.m_bShaderReloadRequired = true;
}

void WaterSettingsBlock::InitGUI(TwBar * bar)
{
	SettingsBlock::InitGUI(bar);
	TwAddButton(bar, "Reload water shaders", ReloadWaterShadersCallBack, nullptr, "group=Water");
}
