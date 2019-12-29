#pragma once
#include "D3D1X2DTexture.h"
class CD3D1X2DRenderTarget :
    public CD3D1X2DTexture, public ID3D1XRenderTargetViewable
{
public:
    CD3D1X2DRenderTarget( RwRaster* parent );
    ~CD3D1X2DRenderTarget();
    void SetDebugName( const std::string& name );
    ID3D11RenderTargetView* GetRenderTargetView() const { return m_pRenderTargetView; }
    void Reallocate();
private:
    ID3D11RenderTargetView * m_pRenderTargetView;
};

