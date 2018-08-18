#include "stdafx.h"
#include "D3D11RenderTargetCache.h"
#include "../../Definitions.h"

RHEngine::D3D11RenderTargetCache::D3D11RenderTargetCache() noexcept
{
	for (unsigned int i = 0; i < 8; i++)
		m_aRenderTargetViews[i] = nullptr;
	m_pDepthStencilView = nullptr;
}

void RHEngine::D3D11RenderTargetCache::SetRenderTargets(const std::unordered_map<int, void*>& renderTargets, void * depthStencilView)
{
	SetRenderTargets(renderTargets);
	SetDepthStencilTarget(depthStencilView);
}

void RHEngine::D3D11RenderTargetCache::SetRenderTargets(const std::unordered_map<int, void*>& renderTargets)
{
	for (auto el : renderTargets)
	{
		if (m_aRenderTargetViews[el.first] != el.second)
		{
			m_aRenderTargetViews[el.first] = reinterpret_cast<ID3D11RenderTargetViewable*>(el.second);
			MakeDirty();
		}
	}
}

void RHEngine::D3D11RenderTargetCache::SetDepthStencilTarget(void * depthStencilView)
{
	if (depthStencilView != m_pDepthStencilView)
	{
		m_pDepthStencilView = reinterpret_cast<ID3D11DepthStencilViewable*>(depthStencilView);
		MakeDirty();
	}
}

void RHEngine::D3D11RenderTargetCache::OnFlush(void* deviceObject)
{
	ID3D11DeviceContext* context = reinterpret_cast<ID3D11DeviceContext*>(deviceObject);
	if (context) 
	{
		ID3D11RenderTargetView* rtvArray[8];
		for (unsigned int i = 0; i < 8; i++)
			rtvArray[i] = m_aRenderTargetViews[i] ? m_aRenderTargetViews[i]->GetRenderTargetView() : nullptr;
		context->OMSetRenderTargets(8, rtvArray, m_pDepthStencilView ? m_pDepthStencilView->GetDepthStencilView() : nullptr);
	}
    context->Release();
}
