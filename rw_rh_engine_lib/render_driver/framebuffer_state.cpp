//
// Created by peter on 16.02.2021.
//

#include "framebuffer_state.h"
#include <Engine/Common/IDeviceState.h>
#include <Engine/Common/ISwapchain.h>

namespace rh::rw::engine
{

FramebufferState::FramebufferState( rh::engine::IDeviceState &device )
{
    using namespace rh::engine;
    for ( auto &frame_res : ResourceCache )
    {
        frame_res.mCmdBuffer = device.CreateCommandBuffer();
        frame_res.mImageAquire =
            device.CreateSyncPrimitive( SyncPrimitiveType::GPU );
        frame_res.mRenderExecute =
            device.CreateSyncPrimitive( SyncPrimitiveType::GPU );
        frame_res.mBufferIsRecorded = false;
    }
}

FramebufferState::~FramebufferState()
{
    for ( auto &frame_res : ResourceCache )
    {
        delete frame_res.mCmdBuffer;
        delete frame_res.mImageAquire;
        delete frame_res.mRenderExecute;
    }
}

PerFrameResources &FramebufferState::CurrentFrameResources()
{
    return ResourceCache[Id];
}
void FramebufferState::NextFrame() { Id = ( Id + 1 ) % CacheSize; }
} // namespace rh::rw::engine