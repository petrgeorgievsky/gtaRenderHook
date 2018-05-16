#pragma once
#include "D3D1X2DTexture.h"
class CD3D1X2DRenderTarget :
	public CD3D1X2DTexture
{
public:
	CD3D1X2DRenderTarget(RwRaster* parent);
	~CD3D1X2DRenderTarget();
	void SetDebugName(const std::string& name);
private:
	ID3D11RenderTargetView * m_pRenderTargetView;
};

