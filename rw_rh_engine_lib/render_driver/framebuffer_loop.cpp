//
// Created by peter on 16.02.2021.
//

#include "framebuffer_loop.h"
#include "frame_renderer.h"

#include <Engine/Common/ISwapchain.h>

namespace rh::rw::engine
{

FramebufferLoop::FramebufferLoop( const FramebufferLoopCreateInfo &info )
    : Device( info.Device ), Window( info.Window ), Renderer( info.Renderer ),
      FramebufferState( info.Device )
{
}

void FramebufferLoop::Run( const FrameState &frame_state )
{
    // Get swapchain
    auto [swap_chain, resize] = Window.GetSwapchain();

    // Handle window change
    if ( resize )
        Renderer.OnResize( Window.GetWindowParams() );

    // Acquire display image
    auto &frame_res = FramebufferState.CurrentFrameResources();
    auto  frame     = swap_chain->GetAvaliableFrame( frame_res.mImageAquire );

    // Record scene to command buffer
    auto dispatch = Renderer.Render( frame_state, frame_res.mCmdBuffer, frame );
    auto to_signal = !dispatch.empty() ? dispatch.back().mToSignalDep : nullptr;
    dispatch.push_back( { frame_res.mCmdBuffer,
                          { frame_res.mImageAquire },
                          frame_res.mRenderExecute } );
    if ( to_signal )
        dispatch.back().mWaitForDep.push_back( to_signal );

    // Send to GPU
    Device.DispatchToGPU( dispatch );

    frame_res.mBufferIsRecorded = true;

    // Release frame
    swap_chain->Present( frame.mImageId, frame_res.mRenderExecute );

    // TODO: Allow non-blocking execution
    // Wait for cmd buffer
    Device.Wait( { frame_res.mCmdBuffer->ExecutionFinishedPrimitive() } );

    FramebufferState.NextFrame();
}
} // namespace rh::rw::engine
