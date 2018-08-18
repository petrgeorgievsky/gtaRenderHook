#include "stdafx.h"
#include "D3D11BackBuffer.h"
#include "../D3D11Common.h"

RHEngine::D3D11BackBuffer::D3D11BackBuffer(ID3D11Device *device, IDXGISwapChain* swapChain)
{
	ID3D11Texture2D* backBufferTexture;
	if (!CALL_D3D_API(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBufferTexture)), 
		TEXT("Failed to get back buffer texture.")))
		return;
    if (backBufferTexture == nullptr) return;

	if (!CALL_D3D_API(device->CreateRenderTargetView(backBufferTexture, nullptr, &m_pRenderTargetView), 
		TEXT("Failed to create render target view for backbuffer texture.")))
		return;

	backBufferTexture->Release();
}

RHEngine::D3D11BackBuffer::~D3D11BackBuffer()
{
	if (m_pRenderTargetView) {
		m_pRenderTargetView->Release();
		m_pRenderTargetView = nullptr;
	}
}

ID3D11RenderTargetView * RHEngine::D3D11BackBuffer::GetRenderTargetView() const
{
	return m_pRenderTargetView;
}
