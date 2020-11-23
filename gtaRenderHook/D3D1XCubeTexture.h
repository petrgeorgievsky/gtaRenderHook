#pragma once
#include "D3D1XBaseTexture.h"
class CD3D1XCubeTexture :
    public CD3D1XBaseTexture, public ID3D1XShaderViewable
{
public:
    CD3D1XCubeTexture( RwRaster* parent, bool createMipMaps );
    ~CD3D1XCubeTexture();
    ID3D11ShaderResourceView* GetShaderResourceView() const override { return m_pShaderView; }
private:
    ID3D11ShaderResourceView * m_pShaderView = nullptr;
};

