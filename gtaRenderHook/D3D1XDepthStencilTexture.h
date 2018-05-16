#pragma once
#include "D3D1X2DTexture.h"
class CD3D1XDepthStencilTexture final :
	public CD3D1X2DTexture
{
public:
	CD3D1XDepthStencilTexture(RwRaster* parent);
	~CD3D1XDepthStencilTexture();
	void SetDebugName(const std::string& name);
	ID3D11DepthStencilView* GetDepthStencilView() const { return m_pDepthStencilView; }
private:
	ID3D11DepthStencilView* m_pDepthStencilView = nullptr;
};

