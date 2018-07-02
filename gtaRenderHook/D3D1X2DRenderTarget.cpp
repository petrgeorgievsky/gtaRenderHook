// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "stdafx.h"
#include "D3D1X2DRenderTarget.h"
#include "D3DSpecificHelpers.h"
#include "RwD3D1XEngine.h"
#include "D3D1XEnumParser.h"
#include "D3DRenderer.h"

CD3D1X2DRenderTarget::CD3D1X2DRenderTarget(RwRaster* parent):
	CD3D1X2DTexture(parent, D3D11_BIND_RENDER_TARGET, "2DRenderTargetTexture", false, true, true)
{
	RwD3D1XRaster* d3dRaster = GetD3D1XRaster(m_pParent);
	ID3D11Device* dev = GET_D3D_DEVICE;
	D3D11_RENDER_TARGET_VIEW_DESC descRTSV{};
	descRTSV.Format = d3dRaster->format;
	descRTSV.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	descRTSV.Texture2D.MipSlice = 0;
	if (!CALL_D3D_API(dev->CreateRenderTargetView(m_pTextureResource, &descRTSV, &m_pRenderTargetView),
		"Failed to create render target view."))
		return;
}


CD3D1X2DRenderTarget::~CD3D1X2DRenderTarget()
{
	if (m_pRenderTargetView) {
		m_pRenderTargetView->Release();
		m_pRenderTargetView = nullptr;
	}
}

void CD3D1X2DRenderTarget::SetDebugName(const std::string & name)
{
	CD3D1X2DTexture::SetDebugName(name);
	if (m_pRenderTargetView)
		g_pDebug->SetD3DName(m_pRenderTargetView, name + "(" + m_resourceTypeName + ", RenderTargetView)");
}

void CD3D1X2DRenderTarget::Reallocate()
{
	// Release allocated resources
	if (m_pRenderTargetView) {
		m_pRenderTargetView->Release();
		m_pRenderTargetView = nullptr;
	}
	// Realloc resources
	CD3D1X2DTexture::Reallocate();

	// Recreate views
	RwD3D1XRaster* d3dRaster = GetD3D1XRaster(m_pParent);
	ID3D11Device* dev = GET_D3D_DEVICE;

	D3D11_RENDER_TARGET_VIEW_DESC descRTSV{};
	descRTSV.Format = d3dRaster->format;
	descRTSV.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	descRTSV.Texture2D.MipSlice = 0;

	if (!CALL_D3D_API(dev->CreateRenderTargetView(m_pTextureResource, &descRTSV, &m_pRenderTargetView),
		"Failed to create render target view."))
		return;
}
