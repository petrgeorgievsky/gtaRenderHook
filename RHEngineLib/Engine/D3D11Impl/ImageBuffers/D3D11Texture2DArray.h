#pragma once
#include "ImageBuffer.h"

#include <d3d11_3.h>
namespace rh::engine {

struct D3D11Texture2DArrayCreateInfo
{
    unsigned int width;
    unsigned int height;
    unsigned int mipCount;
    unsigned int imageCount;
    DXGI_FORMAT format;
    bool allowUnorderedAccessUsage;
    bool allowShaderUsage;
    bool allowRenderTargetUsage;
};

/**
 * @brief D3D11 2D Texture implementation,
 * holds a texture resource and it's views
 *
 */
class D3D11Texture2DArray : public D3D11BindableResource
{
public:
    /**
   * @brief Construct a new D3D112DTexture object
   *
   * @param device - logical device used to allocate memory for graphics
   * resources
   * @param createInfo - texture info
   */
    D3D11Texture2DArray( ID3D11Device *device, const D3D11Texture2DArrayCreateInfo &createInfo );

    /**
   * @brief Destroy the D3D112DTexture object
   *
   */
    ~D3D11Texture2DArray() override;

    ID3D11RenderTargetView *GetRenderTargetView() override;

    ID3D11UnorderedAccessView *GetUnorderedAccessView() override;

    ID3D11ShaderResourceView *GetShaderResourceView() override;
    void *GetImplResource() override;

private:
    D3D11Texture2DArrayCreateInfo m_createInfo{};
    bool m_bLoaded = false;
    ID3D11Texture2D *m_pTexture = nullptr;
    ID3D11UnorderedAccessView *m_pUnorderedAccessView = nullptr;
    ID3D11RenderTargetView *m_pRenderTargetView = nullptr;
    ID3D11ShaderResourceView *m_pShaderResourceView = nullptr;
};
}; // namespace rh::engine
