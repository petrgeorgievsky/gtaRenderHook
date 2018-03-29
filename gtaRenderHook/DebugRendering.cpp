#include "stdafx.h"
#include "DebugRendering.h"

std::list<DebugRenderObject*> DebugRendering::m_aDebugObjects{};

DebugRendering::DebugRendering()
{
}


DebugRendering::~DebugRendering()
{
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
