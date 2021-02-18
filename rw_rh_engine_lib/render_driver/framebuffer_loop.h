//
// Created by peter on 16.02.2021.
//

#pragma once
#include "data_desc/frame_info.h"
#include "framebuffer_state.h"
#include <utility>

namespace rh::rw::engine
{
class IFrameRenderer;

struct FramebufferLoopCreateInfo
{
    // dependencies
    rh::engine::IDeviceState &Device;
    rh::engine::IWindow &     Window;
    IFrameRenderer &          Renderer;
};

class FramebufferLoop
{
  public:
    FramebufferLoop( const FramebufferLoopCreateInfo &info );
    void Run( const FrameState &frame_state );

  private:
    rh::engine::IDeviceState &Device;
    rh::engine::IWindow &     Window;
    IFrameRenderer &          Renderer;
    FramebufferState          FramebufferState;
};

} // namespace rh::rw::engine