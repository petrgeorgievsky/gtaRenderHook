#include "stdafx.h"
#include "D3D1XTexture.h"
#include "D3DRenderer.h"
#include "CDebug.h"
CD3D1XTexture::CD3D1XTexture(CD3DRenderer* pRenderer, RwRaster* pParent, bool mipMaps) : m_pRenderer{ pRenderer }, m_pParent{ pParent }
{
	RwD3D1XRaster* d3dRaster = GetD3D1XRaster(m_pParent);
	D3D11_TEXTURE2D_DESC desc {};
	desc.Width = m_pParent->width;
	desc.Height = m_pParent->height;
	desc.ArraySize = 1;
	desc.Format = d3dRaster->format;
	ID3D11Device* dev = m_pRenderer->getDevice();
	switch (m_pParent->cType)
	{
	case rwRASTERTYPENORMAL:
	case rwRASTERTYPETEXTURE:
		m_type = eD3D1XTextureType::TT_Texture;
		{
			desc.MipLevels = mipMaps ? 0 : 1;
			desc.SampleDesc.Quality = 0;
			desc.SampleDesc.Count = 1;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			desc.CPUAccessFlags = 0;
			desc.MiscFlags = 0;

			if (FAILED(dev->CreateTexture2D(&desc, NULL, &m_pTexture)))
				g_pDebug->printError("Failed to create texture");
			desc.Usage = D3D11_USAGE_STAGING;
			desc.BindFlags = 0;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ|D3D11_CPU_ACCESS_WRITE;
			if (FAILED(dev->CreateTexture2D(&desc, NULL, &m_pStagingTexture)))
				g_pDebug->printError("Failed to create staging texture");
			
			D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
			SRVDesc.Format = d3dRaster->format;
			SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			SRVDesc.Texture2D.MipLevels = UINT32_MAX;
			if (FAILED(dev->CreateShaderResourceView(m_pTexture, &SRVDesc, &m_shaderRV)))
				g_pDebug->printError("Failed to create shader resource view");
		}
		break;
	case rwRASTERTYPEZBUFFER:
		m_type = eD3D1XTextureType::TT_DepthStencil;
		{
			desc.MipLevels = 1;
			desc.SampleDesc.Quality = 0;
			desc.SampleDesc.Count = 1;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			desc.CPUAccessFlags = 0;
			desc.MiscFlags = 0;

			if (FAILED(dev->CreateTexture2D(&desc, NULL, &m_pTexture)))
				g_pDebug->printError("Failed to create texture");

			D3D11_DEPTH_STENCIL_VIEW_DESC descDSV{};
			descDSV.Format = d3dRaster->format;
			descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			descDSV.Texture2D.MipSlice = 0;
			if (FAILED(dev->CreateDepthStencilView(m_pTexture, &descDSV, &m_depthStencilRV)))
				g_pDebug->printError("Failed to create shader resource view");
		}
		break;
	case rwRASTERTYPECAMERATEXTURE:
		m_type = eD3D1XTextureType::TT_RenderTarget;
		{
			desc.MipLevels = 1;
			desc.SampleDesc.Quality = 0;
			desc.SampleDesc.Count = 1;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			desc.CPUAccessFlags = 0;
			desc.MiscFlags = 0;

			if (FAILED(dev->CreateTexture2D(&desc, NULL, &m_pTexture)))
				g_pDebug->printError("Failed to create texture");

			D3D11_RENDER_TARGET_VIEW_DESC descRTSV{};
			descRTSV.Format = d3dRaster->format;
			descRTSV.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			descRTSV.Texture2D.MipSlice = 0;
			if (FAILED(dev->CreateRenderTargetView(m_pTexture, &descRTSV, &m_renderTargetRV)))
				g_pDebug->printError("Failed to create render target resource view");

			D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
			SRVDesc.Format = d3dRaster->format;
			SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			SRVDesc.Texture2D.MipLevels = UINT32_MAX;
			if (FAILED(dev->CreateShaderResourceView(m_pTexture, &SRVDesc, &m_shaderRV)))
				g_pDebug->printError("Failed to create shader resource view");
		}
		break;
	case rwRASTERTYPECAMERA:
		m_type = eD3D1XTextureType::TT_RenderTarget;
		// Create a render target view
		ID3D11Texture2D* pBackBuffer = nullptr;
		if (FAILED(m_pRenderer->getSwapChain()->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer))))
			g_pDebug->printError("Failed to get back buffer");

		if (FAILED(dev->CreateRenderTargetView(pBackBuffer, nullptr, &m_renderTargetRV)))
			g_pDebug->printError("Failed to create render target resource view");
		pBackBuffer->Release();
		break;
	}
}


CD3D1XTexture::~CD3D1XTexture()
{
	if (m_pTexture)
		m_pTexture->Release();
	switch (m_type)
	{
	case eD3D1XTextureType::TT_Texture:
		if (m_shaderRV)
			m_shaderRV->Release();
		if (m_pStagingTexture)
			m_pStagingTexture->Release();
		break;
	case eD3D1XTextureType::TT_RenderTarget:
		if (m_shaderRV)
			m_shaderRV->Release();
		if (m_renderTargetRV)
			m_renderTargetRV->Release();
		break;
	case eD3D1XTextureType::TT_DepthStencil:
		if (m_depthStencilRV)
			m_depthStencilRV->Release();
		break;
	default:
		break;
	}
}

void* CD3D1XTexture::LockToRead()
{
	m_mappedSubRes={};
	m_pRenderer->getContext()->CopyResource(m_pStagingTexture, m_pTexture);
	m_pRenderer->getContext()->Map(m_pStagingTexture, 0,D3D11_MAP_READ_WRITE,0, &m_mappedSubRes);
	m_isLockedToRead = true;
	m_pParent->stride = m_mappedSubRes.RowPitch;
	return m_mappedSubRes.pData;
}

void CD3D1XTexture::UnlockFromRead()
{
	m_isLockedToRead = false;
	m_pRenderer->getContext()->Unmap(m_pStagingTexture, 0);
	m_pRenderer->getContext()->CopyResource(m_pTexture, m_pStagingTexture);
	//if (m_pLockTexture)
	//	m_pLockTexture->Release();
}

