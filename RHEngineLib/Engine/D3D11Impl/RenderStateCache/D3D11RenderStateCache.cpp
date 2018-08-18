#include "stdafx.h"
#include "D3D11RenderStateCache.h"


void RHEngine::D3D11RenderStateCache::FlushRenderTargets(ID3D11DeviceContext * context)
{
	m_rtCache.Flush(context);
}

void RHEngine::D3D11RenderStateCache::Flush(ID3D11DeviceContext * context)
{
	FlushRenderTargets(context);
}
