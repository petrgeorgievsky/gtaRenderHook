//
// Created by peter on 13.05.2021.
//

#pragma once
#include <Engine/Common/ICommandBuffer.h>
#include <Engine/Common/IImageBuffer.h>
#include <Engine/Common/IImageView.h>
#include <Engine/Common/ScopedPtr.h>
#include <rendering_loop/ray_tracing/utils.h>

namespace rh::rw::engine
{
class RenderTextureBuffer
{
  public:
    rh::engine::ScopedPointer<rh::engine::IImageBuffer> Image;
    rh::engine::ScopedPointer<rh::engine::IImageView>   View;
    rh::engine::ImageLayout CurrentLayout = rh::engine::ImageLayout::Undefined;

    rh::engine::ImageMemoryBarrierInfo
    SetLayout( rh::engine::ImageLayout new_layout )
    {
        auto barrier =
            GetLayoutTransformBarrier( Image, CurrentLayout, new_layout );
        CurrentLayout = new_layout;
        return barrier;
    }
};
} // namespace rh::rw::engine