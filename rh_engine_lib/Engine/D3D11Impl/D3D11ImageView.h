#pragma once
#include "Engine/Common/IImageView.h"
#include "Engine/Common/types/image_buffer_type.h"

// d3d11 struct forwards:
struct ID3D11Device;
struct ID3D11Resource;
struct ID3D11View;

namespace rh::engine
{
enum class ImageViewType
{
    Unknown,
    RenderTargetView,
    DepthStencilView,
    UnorderedAccessView,
    ShaderView
};

struct D3D11ImageViewCreateParams
{
    // Dependencies..
    ID3D11Device *  mDevice;
    ID3D11Resource *mResource;
    int32_t         mUsage;
};
struct D3D11ImageViewCreateInfo : ImageViewCreateInfo
{
    // Dependencies..
    ID3D11Device *mDevice;
};

class D3D11ImageView : public IImageView
{
  public:
    D3D11ImageView( const D3D11ImageViewCreateParams &create_params );
    D3D11ImageView( const D3D11ImageViewCreateInfo &create_params );
    ~D3D11ImageView() override;
    ID3D11View *GetSRV() { return mSRView; }
    ID3D11View *GetUAV() { return mUAView; }
    ID3D11View *GetRTV() { return mRTView; }
    ID3D11View *GetDSV() { return mDSView; }

  private:
    ID3D11View *mRTView = nullptr;
    ID3D11View *mDSView = nullptr;
    ID3D11View *mUAView = nullptr;
    ID3D11View *mSRView = nullptr;
};
} // namespace rh::engine
