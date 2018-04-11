#pragma once
#include "DebugRenderObject.h"
// Debug information rendering class.
class CD3D1XShader;
class DebugRendering
{
public:
	static void Init();
	static void Shutdown();
	static void AddToRenderList(DebugRenderObject*);
	static void RemoveFromRenderList(DebugRenderObject*);
	static void ResetList();
	static void Render();
	static void RenderRaster(RwRaster*);
private:
	static std::list<DebugRenderObject*> m_aDebugObjects;
	static CD3D1XShader*		m_pRenderRasterPS;
};

