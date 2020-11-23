#include "D3D11Framebuffer.h"
using namespace rh::engine;

D3D11Framebuffer::D3D11Framebuffer(
    const FrameBufferCreateParams &create_params )
{
    mInfo       = { create_params.width, create_params.height };
    mImageViews = std::vector( create_params.imageViews.begin(),
                               create_params.imageViews.end() );
}

D3D11Framebuffer::~D3D11Framebuffer() {}

const FrameBufferInfo &D3D11Framebuffer::GetInfo() const { return mInfo; }
IImageView *           D3D11Framebuffer::GetImageView( uint8_t id )
{
    return mImageViews[id];
}