//
// Created by peter on 17.02.2021.
//

#pragma once

namespace rh::engine
{
class IDeviceState;
class IWindow;
class IImageBuffer;
class IImageView;
} // namespace rh::engine

namespace rh::rw::engine
{
struct RasterData
{
    rh::engine::IImageBuffer *mImageBuffer;
    rh::engine::IImageView *  mImageView;
    void                      Release();
};
} // namespace rh::rw::engine