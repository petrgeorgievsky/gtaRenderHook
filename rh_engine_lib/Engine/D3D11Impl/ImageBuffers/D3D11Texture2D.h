#pragma once
#include "ImageBuffer.h"
#include <vector>
#include <dxgiformat.h>

// d3d11 struct forwards:
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11Texture2D;
struct D3D11_SUBRESOURCE_DATA;

namespace rh::engine {

struct D3D11Texture2DCreateInfo
{
    unsigned int width;
    unsigned int height;
    unsigned int mipCount;
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
class D3D11Texture2D : public D3D11BindableResource
{
public:
    /**
   * @brief Construct a new D3D112DTexture object
   *
   * @param device - logical device used to allocate memory for graphics
   * resources
   * @param createInfo - texture info
   */
    D3D11Texture2D( ID3D11Device *device,
                    const D3D11Texture2DCreateInfo &createInfo,
                    std::vector<D3D11_SUBRESOURCE_DATA> &&initialData );

    /**
   * @brief Destroy the D3D112DTexture object
   *
   */
    ~D3D11Texture2D() override;

    ID3D11RenderTargetView *GetRenderTargetView() override;

    ID3D11UnorderedAccessView *GetUnorderedAccessView() override;

    ID3D11ShaderResourceView *GetShaderResourceView() override;

    [[nodiscard]] const D3D11Texture2DCreateInfo &GetTextureInfo() const;

    void *GetImplResource() override;

private:
    void Load( ID3D11Device *device, std::vector<D3D11_SUBRESOURCE_DATA> &&initialData );

    D3D11Texture2DCreateInfo m_createInfo{};
    bool m_bLoaded = false;
    ID3D11Texture2D *m_pTexture = nullptr;
    ID3D11UnorderedAccessView *m_pUnorderedAccessView = nullptr;
    ID3D11RenderTargetView *m_pRenderTargetView = nullptr;
    ID3D11ShaderResourceView *m_pShaderResourceView = nullptr;
};
} // namespace rh::engine
