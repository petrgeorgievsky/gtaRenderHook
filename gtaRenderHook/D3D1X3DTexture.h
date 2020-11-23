#pragma once
#include "D3D1XBaseTexture.h"


class CD3D1X3DTexture : public CD3D1XBaseTexture, public ID3D1XShaderViewable
{
  public:
    CD3D1X3DTexture( RwRaster *parent, const D3D11_SUBRESOURCE_DATA* resourse_data = nullptr );
    ~CD3D1X3DTexture();
    ID3D11ShaderResourceView *GetShaderResourceView() const override
    {
        return m_pShaderView;
    }
  private:
    UINT m_nWidth=0;
    UINT m_nHeight=0;
    UINT                      m_nDepth      = 0;
    ID3D11ShaderResourceView *m_pShaderView = nullptr;
};
