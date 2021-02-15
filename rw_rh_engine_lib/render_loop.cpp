#include "render_loop.h"
#include <Engine/Common/ISwapchain.h>
#include <Engine/VulkanImpl/VulkanBottomLevelAccelerationStructure.h>
#include <functional>
#include <rendering_loop/ray_tracing/RayTracingRenderer.h>
#include <rw_engine/rh_backend/material_backend.h>
#include <rw_engine/system_funcs/rw_device_system_globals.h>

namespace rh::rw::engine
{
std::unique_ptr<CameraState>    EngineState::gCameraState   = nullptr;
std::shared_ptr<IFrameRenderer> EngineState::gFrameRenderer = nullptr;

CameraState::CameraState( rh::engine::IDeviceState &device )
{
    using namespace rh::engine;
    for ( auto &frame_res : mFrameResourceCache )
    {
        frame_res.mCmdBuffer = device.CreateCommandBuffer();
        frame_res.mImageAquire =
            device.CreateSyncPrimitive( SyncPrimitiveType::GPU );
        frame_res.mRenderExecute =
            device.CreateSyncPrimitive( SyncPrimitiveType::GPU );
        frame_res.mBufferIsRecorded = false;
    }
}

CameraState::~CameraState()
{
    for ( auto &frame_res : mFrameResourceCache )
    {
        delete frame_res.mCmdBuffer;
        delete frame_res.mImageAquire;
        delete frame_res.mRenderExecute;
    }
}
PerFrameResources &CameraState::CurrentFrameResources()
{
    return mFrameResourceCache[mFrameId];
}
void CameraState::NextFrame()
{
    mFrameId = ( mFrameId + 1 ) % gFrameResourceCacheSize;
}
} // namespace rh::rw::engine