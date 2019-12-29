#pragma once
#include "Engine/Common/IFrameBuffer.h"
namespace rh::engine
{
class D3D11Framebuffer : public IFrameBuffer
{
  public:
    D3D11Framebuffer( const FrameBufferCreateParams &create_params );
    ~D3D11Framebuffer() override;
    const FrameBufferInfo &GetInfo() const override;
    IImageView *           GetImageView( uint8_t id );

  private:
    FrameBufferInfo           mInfo;
    std::vector<IImageView *> mImageViews;
};
} // namespace rh::engine
