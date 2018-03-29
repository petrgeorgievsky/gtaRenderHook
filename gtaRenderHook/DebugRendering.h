#pragma once
#include "DebugRenderObject.h"
// Debug information rendering class.
class DebugRendering
{
public:
	DebugRendering();
	~DebugRendering();
	static void AddToRenderList(DebugRenderObject*);
	static void RemoveFromRenderList(DebugRenderObject*);
	static void ResetList();
	static void Render();
private:
	static std::list<DebugRenderObject*> m_aDebugObjects;
};

