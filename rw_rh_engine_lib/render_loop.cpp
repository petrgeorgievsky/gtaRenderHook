#include "render_loop.h"
#include <Engine/Common/ISwapchain.h>
#include <Engine/VulkanImpl/VulkanBottomLevelAccelerationStructure.h>
#include <functional>
#include <rendering_loop/ray_tracing/RayTracingRenderer.h>
#include <rw_engine/rh_backend/material_backend.h>
#include <rw_engine/system_funcs/rw_device_system_globals.h>

namespace rh::rw::engine
{
Im2DClientGlobals               EngineClient::gIm2DGlobals{};
BackendRendererClient           EngineClient::gRendererGlobals{};
Im3DClient                      EngineClient::gIm3DGlobals{};
SkinRendererClient              EngineClient::gSkinRendererGlobals{};
std::unique_ptr<CameraState>    EngineState::gCameraState   = nullptr;
std::shared_ptr<IFrameRenderer> EngineState::gFrameRenderer = nullptr;

void ExecuteRender()
{
    gRenderClient->GetTaskQueue().ExecuteTask(
        SharedMemoryTaskType::RENDER,
        []( MemoryWriter &&memory_writer ) {
            SerializeSceneGraph( memory_writer );
        },
        []( MemoryReader &&memory_reader ) {
            EngineClient::gIm2DGlobals.Flush();
            EngineClient::gIm3DGlobals.Flush();
            EngineClient::gRendererGlobals.Flush();
            EngineClient::gSkinRendererGlobals.Flush();
        } );
}

void Render( SceneInfo *scene )
{
    auto &window    = gRenderDriver->GetMainWindow();
    auto &device    = gRenderDriver->GetDeviceState();
    auto &cam_state = *EngineState::gCameraState;
    auto &renderer  = *EngineState::gFrameRenderer;

    // Get swapchain
    auto [swap_chain, resize] = window.GetSwapchain();

    // Handle window change
    if ( resize )
        renderer.OnResize( window.GetWindowParams() );

    // Acquire display image
    auto &frame_res = cam_state.CurrentFrameResources();
    auto  frame     = swap_chain->GetAvaliableFrame( frame_res.mImageAquire );

    // Record scene to command buffer
    auto dispatch  = renderer.Render( scene, frame_res.mCmdBuffer, frame );
    auto to_signal = !dispatch.empty() ? dispatch.back().mToSignalDep : nullptr;
    dispatch.push_back( { frame_res.mCmdBuffer,
                          { frame_res.mImageAquire },
                          frame_res.mRenderExecute } );
    if ( to_signal )
        dispatch.back().mWaitForDep.push_back( to_signal );
    // Send to GPU
    device.DispatchToGPU( dispatch );

    frame_res.mBufferIsRecorded = true;

    // Release frame
    swap_chain->Present( frame.mImageId, frame_res.mRenderExecute );

    // TODO: Allow non-blocking execution
    // Wait for cmd buffer
    device.Wait( { frame_res.mCmdBuffer->ExecutionFinishedPrimitive() } );

    cam_state.NextFrame();

    gRenderDriver->GetResources().GC();
}

void InitRenderEvents()
{
    // EngineState::gRendererServerGlobals = new BackendRenderer();
    gRenderDriver->GetTaskQueue().RegisterTask(
        SharedMemoryTaskType::RENDER,
        std::make_unique<SharedMemoryTask>( []( void *memory ) {
            // execute
            void *current_memory_ptr = memory;
            auto  frame_info = static_cast<FrameInfo *>( current_memory_ptr );

            // Get scene pointers TODO: Make it smarter
            SceneInfo scene{};
            scene.mFrameInfo = frame_info;
            scene.mFrontendRenderBlock =
                static_cast<char *>( memory ) + sizeof( FrameInfo );

            auto marker =
                *static_cast<uint64_t *>( scene.mFrontendRenderBlock );

            scene.mIm3DRenderBlock = static_cast<char *>( memory ) + marker;
            marker = *static_cast<uint64_t *>( scene.mIm3DRenderBlock );

            scene.mSkinMeshRenderBlock = static_cast<char *>( memory ) + marker;
            marker = *static_cast<uint64_t *>( scene.mSkinMeshRenderBlock );

            scene.mSceneMeshRenderBlock =
                static_cast<char *>( memory ) + marker;

            Render( &scene );
        } ) );
}

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