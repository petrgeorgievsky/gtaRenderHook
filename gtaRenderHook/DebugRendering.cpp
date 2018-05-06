// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "stdafx.h"
#include "DebugRendering.h"
#include "D3D1XShader.h"
#include "D3D1XStateManager.h"
#include "FullscreenQuad.h"

std::list<DebugRenderObject*> DebugRendering::m_aDebugObjects{};
CD3D1XShader* DebugRendering::m_pRenderRasterPS=nullptr;

void DebugRendering::Init()
{
	const auto shader_path = "shaders/debug.hlsl";

	m_pRenderRasterPS = new CD3D1XPixelShader(shader_path, "RenderRasterPS");
}


void DebugRendering::Shutdown()
{
	delete m_pRenderRasterPS;
}

void DebugRendering::AddToRenderList(DebugRenderObject* renderObject)
{
	m_aDebugObjects.push_back(renderObject);
}

void DebugRendering::RemoveFromRenderList(DebugRenderObject * renderObject)
{
	m_aDebugObjects.remove(renderObject);
}

void DebugRendering::ResetList()
{
	for (auto debugObject : m_aDebugObjects)
		delete debugObject;
	m_aDebugObjects.clear();
}

void DebugRendering::Render()
{
	for (auto debugObject : m_aDebugObjects)
		debugObject->Render();
}

void DebugRendering::RenderRaster(RwRaster * raster)
{
	g_pStateMgr->SetRaster(raster);
	m_pRenderRasterPS->Set();
	CFullscreenQuad::Draw();
	m_pRenderRasterPS->ReSet();
}
