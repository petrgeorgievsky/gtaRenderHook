#pragma once
#include "D3D1XBaseTexture.h"
class CD3D1XBackBufferTexture final :
    public CD3D1XBaseTexture, public ID3D1XRenderTargetViewable
{
public:
    CD3D1XBackBufferTexture( RwRaster* parent );
    ~CD3D1XBackBufferTexture();
    void SetDebugName( const std::string& name );
    ID3D11RenderTargetView* GetRenderTargetView() const { return m_pRenderTargetView; }
private:
    ID3D11RenderTargetView *m_pRenderTargetView{};
};

