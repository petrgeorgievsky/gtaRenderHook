// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "stdafx.h"
#include "D3D1XTexture.h"
#include "D3DRenderer.h"
#include "RwD3D1XEngine.h"
#include "D3D1XShader.h"
#include "CDebug.h"
#include "D3DSpecificHelpers.h"

CD3D1XTexture::CD3D1XTexture(RwRaster* pParent, bool mipMaps,bool hasPalette) : m_pParent{ pParent }, m_hasPalette{ hasPalette }
{
	m_pTexture			= nullptr;
	m_shaderRV			= nullptr;
	m_pStagingTexture	= nullptr;
	m_dataPtr = nullptr;
	m_nWidth = m_pParent->width;
	m_nHeight = m_pParent->height;
	m_mappedSubRes = {};
	for (int i = 0; i < 256; i++)
		m_palette[i] = { 255,255,255,255 };
	
	RwD3D1XRaster* d3dRaster = GetD3D1XRaster(m_pParent);
	ID3D11Device* dev = GET_D3D_DEVICE;
	// Hardcoded 3D texture initialization
	// TODO: Move somewhere else 
	if (d3dRaster->textureFlags == 64) 
	{
		m_type = eD3D1XTextureType::TT_3DTexture;
		D3D11_TEXTURE3D_DESC desc{};
		desc.Width = m_pParent->width;
		desc.Height = m_pParent->height;
		desc.Depth = m_pParent->depth;
		desc.Format = d3dRaster->format;
		desc.MipLevels = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE|D3D11_BIND_UNORDERED_ACCESS| D3D11_BIND_RENDER_TARGET;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		if (!CALL_D3D_API(dev->CreateTexture3D(&desc, NULL, &m_p3DTexture), "Failed to create 3D texture buffer."))
			return;

		g_pDebug->SetD3DName(m_p3DTexture, "CD3D1XTexture::UAV3DTexture");

		D3D11_UNORDERED_ACCESS_VIEW_DESC uavdesc;
		uavdesc.Format = d3dRaster->format;
		uavdesc.Texture3D.MipSlice = 0;
		uavdesc.Texture3D.FirstWSlice = 0;
		uavdesc.Texture3D.WSize = -1;
		uavdesc.ViewDimension = D3D11_UAV_DIMENSION::D3D11_UAV_DIMENSION_TEXTURE3D;

		if (!CALL_D3D_API(dev->CreateUnorderedAccessView(m_p3DTexture, &uavdesc, &m_unorderedAV),"Failed to create 3D unordered access view"))
			return;
		g_pDebug->SetD3DName(m_unorderedAV, "CD3D1XTexture::3dUAV");

		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
		SRVDesc.Format = d3dRaster->format;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
		SRVDesc.Texture3D.MipLevels = UINT32_MAX;
		if (CALL_D3D_API(dev->CreateShaderResourceView(m_p3DTexture, &SRVDesc, &m_shaderRV), "Failed to create 3D shader resource view"))
			g_pDebug->SetD3DName(m_shaderRV, "CD3D1XTexture::3dShaderRV");

		return;
	}
	// Base texture initialization
	D3D11_TEXTURE2D_DESC desc {};
	desc.Width	= m_pParent->width;
	desc.Height = m_pParent->height;
	desc.Format = d3dRaster->format;
	desc.ArraySize = 1;

	switch (m_pParent->cType)
	{
	case rwRASTERTYPENORMAL:
	case rwRASTERTYPETEXTURE:					// Texture with shader resource view and nothing more 
		m_type = eD3D1XTextureType::TT_Texture;
		{
			// base texture buffer
			desc.MipLevels = mipMaps ? max((int)log2(min(desc.Width, desc.Height))-2,0) : 1;
			desc.SampleDesc.Quality = 0;
			desc.SampleDesc.Count = 1;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			desc.CPUAccessFlags = 0;
			desc.MiscFlags = 0;
			if (!CALL_D3D_API(dev->CreateTexture2D(&desc, NULL, &m_pTexture),"Failed to create texture"))
				return;
			g_pDebug->SetD3DName(m_pTexture, "CD3D1XTexture::Texture");

			// shader resource view
			D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
			SRVDesc.Format = d3dRaster->format;
			SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			SRVDesc.Texture2D.MipLevels = UINT32_MAX;

			if (!CALL_D3D_API(dev->CreateShaderResourceView(m_pTexture, &SRVDesc, &m_shaderRV), "Failed to create shader resource view"))
				return;
			g_pDebug->SetD3DName(m_shaderRV, "CD3D1XTexture::ShaderRV");
		}
		break;
	case rwRASTERTYPEZBUFFER:						// Texture used as depth target
		m_type = eD3D1XTextureType::TT_DepthStencil;
		{

			desc.Format = DXGI_FORMAT_R24G8_TYPELESS;
			desc.MipLevels = 1;
			desc.SampleDesc.Quality = 0;
			desc.SampleDesc.Count = 1;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_DEPTH_STENCIL| D3D11_BIND_SHADER_RESOURCE;
			desc.CPUAccessFlags = 0;
			desc.MiscFlags = 0;

			if (!CALL_D3D_API(dev->CreateTexture2D(&desc, NULL, &m_pTexture), "Failed to create depth texture"))
				return;

			g_pDebug->SetD3DName(m_pTexture, "CD3D1XTexture::DSTexture");

			D3D11_DEPTH_STENCIL_VIEW_DESC descDSV{};
			descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			descDSV.Texture2D.MipSlice = 0;

			if (!CALL_D3D_API(dev->CreateDepthStencilView(m_pTexture, &descDSV, &m_depthStencilRV), "Failed to create depth stencil view"))
				return;
			g_pDebug->SetD3DName(m_depthStencilRV, "CD3D1XTexture::DepthStencilRV");

			D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
			SRVDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			SRVDesc.Texture2D.MipLevels = UINT32_MAX;

			if (!CALL_D3D_API(dev->CreateShaderResourceView(m_pTexture, &SRVDesc, &m_shaderRV), "Failed to create shader resource view"))
				return;
			g_pDebug->SetD3DName(m_shaderRV, "CD3D1XTexture::ShaderRV");
		}
		break;
	case rwRASTERTYPECAMERATEXTURE:
		m_type = eD3D1XTextureType::TT_RenderTarget;
		InitCameraTextureRaster(dev, &desc, d3dRaster, mipMaps);
		break;
	case rwRASTERTYPECAMERA:
		m_type = eD3D1XTextureType::TT_RenderTarget;
		InitCameraRaster(dev);
		break;
	}
}

void CD3D1XTexture::InitCameraTextureRaster(ID3D11Device * dev, D3D11_TEXTURE2D_DESC *desc, RwD3D1XRaster * d3dRaster, bool mipmaps)
{
	desc->MipLevels = mipmaps ? 0 : 1;
	desc->SampleDesc.Quality = 0;
	desc->SampleDesc.Count = 1;
	desc->Usage = D3D11_USAGE_DEFAULT;	
	desc->BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	desc->CPUAccessFlags = 0;
	desc->MiscFlags = mipmaps ? D3D11_RESOURCE_MISC_GENERATE_MIPS:0;

	if (!CALL_D3D_API(dev->CreateTexture2D(desc, NULL, &m_pTexture), "Failed to create camera texture"))
		return;
	g_pDebug->SetD3DName(m_pTexture, "CD3D1XTexture::CameraTexTexture");

	D3D11_RENDER_TARGET_VIEW_DESC descRTSV{};
	descRTSV.Format = d3dRaster->format;
	descRTSV.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	descRTSV.Texture2D.MipSlice = 0;
	if (!CALL_D3D_API(dev->CreateRenderTargetView(m_pTexture, &descRTSV, &m_renderTargetRV), "Failed to create render target view."))
		return;
	g_pDebug->SetD3DName(m_renderTargetRV, "CD3D1XTexture::CameraTexRTV");

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
	SRVDesc.Format = d3dRaster->format;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = UINT32_MAX;
	if (!CALL_D3D_API(dev->CreateShaderResourceView(m_pTexture, &SRVDesc, &m_shaderRV), "Failed to create shader resource view"))
		return;
	g_pDebug->SetD3DName(m_shaderRV, "CD3D1XTexture::CameraTexSRV");
}

// Initializes backbuffer(camera) raster.
void CD3D1XTexture::InitCameraRaster(ID3D11Device * dev)
{
	ID3D11Texture2D* pBackBuffer = nullptr;
	
	if (!CALL_D3D_API(GET_D3D_SWAP_CHAIN->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer)), "Failed to get back buffer texture."))
		return;

	if (!CALL_D3D_API(dev->CreateRenderTargetView(pBackBuffer, nullptr, &m_renderTargetRV), "Failed to create render target view."))
		return;

	g_pDebug->SetD3DName(m_renderTargetRV, "CD3D1XTexture::CameraRTV");
	pBackBuffer->Release();
}


CD3D1XTexture::~CD3D1XTexture()
{
	m_pParent = nullptr;
	if (m_shaderRV) {
		m_shaderRV->Release();
		m_shaderRV = nullptr;
	}
	switch (m_type)
	{
	case eD3D1XTextureType::TT_RenderTarget:
		if (m_renderTargetRV) {
			m_renderTargetRV->Release();
			m_renderTargetRV = nullptr;
		}
		break;
	case eD3D1XTextureType::TT_DepthStencil:
		if (m_depthStencilRV) {
			m_depthStencilRV->Release();
			m_depthStencilRV = nullptr;
		}
		break;
	case eD3D1XTextureType::TT_3DTexture:
		if (m_unorderedAV) {
			m_unorderedAV->Release();
			m_unorderedAV = nullptr;
		}
		if (m_p3DTexture) {
			m_p3DTexture->Release();
			m_p3DTexture = nullptr;
		}
		break;
	default:
		break;
	}
	if (m_pStagingTexture) {
		m_pStagingTexture->Release();
		m_pStagingTexture = nullptr;
	}
	if (m_pTexture) {
		m_pTexture->Release();
		m_pTexture = nullptr;
	}
	if (m_dataPtr) {
		free(m_dataPtr);
		m_dataPtr = nullptr;
	}
}

void* CD3D1XTexture::LockToRead()
{
	m_isLockedToRead = true;
	RwD3D1XRaster* d3dRaster = GetD3D1XRaster(m_pParent);
	if (m_pStagingTexture == nullptr) {
		D3D11_TEXTURE2D_DESC desc{};
		desc.Width = m_pParent->width;
		desc.Height = m_pParent->height;
		desc.Format = d3dRaster->format;
		desc.MipLevels = 1;
		desc.SampleDesc.Quality = 0;
		desc.SampleDesc.Count = 1;
		desc.ArraySize = 1;
		desc.Usage = D3D11_USAGE_STAGING;
		desc.BindFlags = 0;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;

		if (!CALL_D3D_API(GET_D3D_DEVICE->CreateTexture2D(&desc, NULL, &m_pStagingTexture), "Failed to create staging texture"))
			return nullptr;
		g_pDebug->SetD3DName(m_pStagingTexture, "CD3D1XTexture::StagingTexture");
	}
	if (!m_hasPalette) {
		m_mappedSubRes = {};
		auto context = GET_D3D_CONTEXT;
		context->CopyResource(m_pStagingTexture, m_pTexture);
		context->Map(m_pStagingTexture, 0, D3D11_MAP_READ_WRITE, 0, &m_mappedSubRes);
		m_pParent->stride = m_mappedSubRes.RowPitch;
		return m_mappedSubRes.pData;
	}
	else {
		m_dataPtr = (BYTE*)malloc(sizeof(BYTE)*m_pParent->width*m_pParent->height);
		return m_dataPtr;
	}
}

void CD3D1XTexture::UnlockFromRead()
{
	auto context = GET_D3D_CONTEXT;
	m_isLockedToRead = false;

	if (m_hasPalette) {
		context->CopyResource(m_pStagingTexture, m_pTexture);
		context->Map(m_pStagingTexture, 0, D3D11_MAP_READ_WRITE, 0, &m_mappedSubRes);
		UINT size = m_pParent->width*m_pParent->height;
		// convert raster data
		int* remappedPtr = (int*)m_mappedSubRes.pData;
		for (size_t i = 0; i < size; i++) {
			auto color = m_palette[m_dataPtr[i]];
			remappedPtr[i] = RWRGBALONG(color.red, color.green, color.blue, color.alpha);
		}
		free(m_dataPtr);
		m_dataPtr = nullptr;
	}
	context->Unmap(m_pStagingTexture, 0);
	context->CopyResource(m_pTexture, m_pStagingTexture);

	if (m_pStagingTexture) {
		m_pStagingTexture->Release();
		m_pStagingTexture = nullptr;
	}
	//if (m_pLockTexture)
	//	m_pLockTexture->Release();
}

void CD3D1XTexture::Resize(UINT newWidth, UINT newHeight)
{
	ID3D11Device* dev = GET_D3D_DEVICE;
	RwD3D1XRaster* d3dRaster = GetD3D1XRaster(m_pParent);
	m_pParent->width = newWidth;
	m_pParent->height = newHeight;
	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = m_pParent->width;
	desc.Height = m_pParent->height;
	desc.Format = d3dRaster->format;
	desc.ArraySize = 1;
	if (m_shaderRV) {
		m_shaderRV->Release();
	}
	if (m_depthStencilRV) {
		m_depthStencilRV->Release();
	}
	if (m_pStagingTexture) {
		m_pStagingTexture->Release();
	}
	if (m_pTexture) {
		m_pTexture->Release();
	}
	m_pTexture = nullptr;
	m_shaderRV = nullptr;
	m_pStagingTexture = nullptr;
	switch (m_type)
	{
	case eD3D1XTextureType::TT_Texture:

		break;
	case eD3D1XTextureType::TT_RenderTarget:
		InitCameraTextureRaster(dev, &desc, d3dRaster, 0);
		break;
	case eD3D1XTextureType::TT_DepthStencil:
	{
		desc.Format = DXGI_FORMAT_R24G8_TYPELESS;
		desc.MipLevels = 1;
		desc.SampleDesc.Quality = 0;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		if (!CALL_D3D_API(dev->CreateTexture2D(&desc, NULL, &m_pTexture), "Failed to create depth texture"))
			return;

		g_pDebug->SetD3DName(m_pTexture, "CD3D1XTexture::DSTexture");

		D3D11_DEPTH_STENCIL_VIEW_DESC descDSV{};
		descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		descDSV.Texture2D.MipSlice = 0;
		if (!CALL_D3D_API(dev->CreateDepthStencilView(m_pTexture, &descDSV, &m_depthStencilRV), "Failed to create depth stencil view"))
			return;
		g_pDebug->SetD3DName(m_depthStencilRV, "CD3D1XTexture::DepthStencilRV");

		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
		SRVDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = UINT32_MAX;

		if (!CALL_D3D_API(dev->CreateShaderResourceView(m_pTexture, &SRVDesc, &m_shaderRV), "Failed to create shader resource view"))
			return;
		g_pDebug->SetD3DName(m_shaderRV, "CD3D1XTexture::ShaderRV");
	}
		break;
	case eD3D1XTextureType::TT_3DTexture:
		break;
	default:
		break;
	}
}

void CD3D1XTexture::Reload()
{
	ID3D11Device* dev = GET_D3D_DEVICE;
	RwD3D1XRaster* d3dRaster = GetD3D1XRaster(m_pParent);
	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = m_pParent->width;
	desc.Height = m_pParent->height;
	desc.Format = d3dRaster->format;
	desc.ArraySize = 1;
	if (m_shaderRV) {
		m_shaderRV->Release();
	}
	if (m_depthStencilRV) {
		m_depthStencilRV->Release();
	}
	if (m_pStagingTexture) {
		m_pStagingTexture->Release();
	}
	if (m_pTexture) {
		m_pTexture->Release();
	}
	m_pTexture = nullptr;
	m_shaderRV = nullptr;
	m_pStagingTexture = nullptr;
	switch (m_type)
	{
	case eD3D1XTextureType::TT_Texture:

		break;
	case eD3D1XTextureType::TT_RenderTarget:
		InitCameraTextureRaster(dev, &desc, d3dRaster, 0);
		break;
	case eD3D1XTextureType::TT_DepthStencil:
	{
		desc.Format = DXGI_FORMAT_R24G8_TYPELESS;
		desc.MipLevels = 1;
		desc.SampleDesc.Quality = 0;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		if (!CALL_D3D_API(dev->CreateTexture2D(&desc, NULL, &m_pTexture),"Failed to create depth texture"))
			return;

		g_pDebug->SetD3DName(m_pTexture, "CD3D1XTexture::DSTexture");

		D3D11_DEPTH_STENCIL_VIEW_DESC descDSV{};
		descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		descDSV.Texture2D.MipSlice = 0;
		if (!CALL_D3D_API(dev->CreateDepthStencilView(m_pTexture, &descDSV, &m_depthStencilRV), "Failed to create depth stencil view"))
			return;
		g_pDebug->SetD3DName(m_depthStencilRV, "CD3D1XTexture::DepthStencilRV");

		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
		SRVDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = UINT32_MAX;

		if (!CALL_D3D_API(dev->CreateShaderResourceView(m_pTexture, &SRVDesc, &m_shaderRV),"Failed to create shader resource view"))
			return;
		g_pDebug->SetD3DName(m_shaderRV, "CD3D1XTexture::ShaderRV");
	}
	break;
	case eD3D1XTextureType::TT_3DTexture:
		break;
	default:
		break;
	}
}


