#pragma once
#include "Engine/Common/IImageView.h"
#include "Engine/Common/types/image_buffer_type.h"
#include <common.h>
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
    ImageViewType   mViewType;
};

class D3D11ImageView : public IImageView
{
  public:
    D3D11ImageView( const D3D11ImageViewCreateParams &create_params );
    ~D3D11ImageView() override;
    operator ID3D11View *() { return mView; }

  private:
    ID3D11View *mView = nullptr;
};
} // namespace rh::engine