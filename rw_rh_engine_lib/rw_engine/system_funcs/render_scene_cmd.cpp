//
// Created by peter on 12.02.2021.
//

#include "render_scene_cmd.h"
#include "rw_device_system_globals.h"
#include <Engine/Common/IDeviceState.h>
#include <Engine/Common/ISwapchain.h>
#include <ipc/shared_memory_queue_client.h>
#include <render_client/render_client.h>
#include <render_driver/render_driver.h>
#include <scene_graph.h>

namespace rh::rw::engine
{

RenderSceneCmd::RenderSceneCmd( SharedMemoryTaskQueue &task_queue )
    : TaskQueue( task_queue )
{
}

void WriteChainBlock( MemoryWriter &                          writer,
                      std::function<void( MemoryWriter & )> &&serialize )
{
    // reserve next block ptr
    auto &next_block_offset = writer.Current<uint64_t>();
    writer.Skip( sizeof( uint64_t ) );
    // store current address
    auto start_pos = writer.Pos();
    // Serialize
    serialize( writer );
    // store next block offset
    next_block_offset = writer.Pos() - start_pos;
}
void *ReadChainBlock( MemoryReader &reader )
{
    auto next_block_offset = *reader.Read<uint64_t>();
    auto block_address     = reader.CurrentAddress();
    reader.Skip( next_block_offset );
    return block_address;
}

void SerializeDrawCalls( MemoryWriter &&writer )
{
    GetCurrentSceneGraph()->mFrameInfo.mFrameId =
        ( GetCurrentSceneGraph()->mFrameInfo.mFrameId + 1 ) % 1000;

    writer.Write( &GetCurrentSceneGraph()->mFrameInfo );
    WriteChainBlock( writer, []( MemoryWriter &w ) {
        gRenderClient->RenderState.Im2D.Serialize( w );
    } );
    WriteChainBlock( writer, []( MemoryWriter &w ) {
        gRenderClient->RenderState.Im3D.Serialize( w );
    } );
    WriteChainBlock( writer, []( MemoryWriter &w ) {
        gRenderClient->RenderState.SkinMeshDrawCalls.Serialize( w );
    } );
    WriteChainBlock( writer, []( MemoryWriter &w ) {
        gRenderClient->RenderState.MeshDrawCalls.Serialize( w );
    } );
}

bool RenderSceneCmd::Invoke()
{
    TaskQueue.ExecuteTask( SharedMemoryTaskType::RENDER, SerializeDrawCalls );
    gRenderClient->RenderState.Im2D.Flush();
    gRenderClient->RenderState.Im3D.Flush();
    gRenderClient->RenderState.MeshDrawCalls.Flush();
    gRenderClient->RenderState.SkinMeshDrawCalls.Flush();
    return true;
}

void Render( rh::engine::IWindow &window, rh::engine::IDeviceState &device,
             const SceneInfo &scene )
{
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
    auto dispatch  = renderer.Render( &scene, frame_res.mCmdBuffer, frame );
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
}

void RenderSceneTaskImpl( void *memory )
{
    using namespace rh::engine;
    assert( gRenderDriver );
    auto &driver = *gRenderDriver;

    MemoryReader reader( memory );

    SceneInfo scene{};
    scene.mFrameInfo            = reader.Read<FrameInfo>();
    scene.mIm2DRenderBlock      = ReadChainBlock( reader );
    scene.mIm3DRenderBlock      = ReadChainBlock( reader );
    scene.mSkinMeshRenderBlock  = ReadChainBlock( reader );
    scene.mSceneMeshRenderBlock = ReadChainBlock( reader );

    Render( driver.GetMainWindow(), driver.GetDeviceState(), scene );
    driver.GetResources().GC();
}

void RenderSceneCmd::RegisterCallHandler( SharedMemoryTaskQueue &task_queue )
{
    task_queue.RegisterTask(
        SharedMemoryTaskType::RENDER,
        std::make_unique<SharedMemoryTask>( RenderSceneTaskImpl ) );
}

} // namespace rh::rw::engine