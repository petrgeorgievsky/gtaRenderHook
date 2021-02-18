//
// Created by peter on 12.02.2021.
//

#include "render_scene_cmd.h"
#include "rw_device_system_globals.h"
#include <Engine/Common/IDeviceState.h>
#include <Engine/Common/ISwapchain.h>
#include <data_desc/frame_info.h>
#include <ipc/shared_memory_queue_client.h>
#include <render_client/render_client.h>
#include <render_driver/gpu_resources/resource_mgr.h>
#include <render_driver/render_driver.h>

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

template <typename deserialize, typename T>
T ReadChainBlock( MemoryReader &reader )
{
    auto next_block_offset = *reader.Read<uint64_t>();
    return deserialize( reader );
}

void SerializeDrawCalls( MemoryWriter &&writer )
{
    auto &state = gRenderClient->RenderState;
    writer.Write( &state.ViewportState );
    writer.Write( &state.SkyState );
    state.Lights.Serialize( writer );
    state.Im2D.Serialize( writer );
    state.Im3D.Serialize( writer );
    state.MeshDrawCalls.Serialize( writer );
    state.SkinMeshDrawCalls.Serialize( writer );
}

bool RenderSceneCmd::Invoke()
{
    TaskQueue.ExecuteTask( SharedMemoryTaskType::RENDER, SerializeDrawCalls );
    gRenderClient->RenderState.Im2D.Flush();
    gRenderClient->RenderState.Im3D.Flush();
    gRenderClient->RenderState.MeshDrawCalls.Flush();
    gRenderClient->RenderState.SkinMeshDrawCalls.Flush();
    gRenderClient->RenderState.Lights.Flush();
    return true;
}

void Render( rh::engine::IWindow &window, rh::engine::IDeviceState &device,
             const FrameState &state )
{
}

void RenderSceneTaskImpl( void *memory )
{
    using namespace rh::engine;
    assert( gRenderDriver );
    auto &driver = *gRenderDriver;

    MemoryReader reader( memory );
    FrameState   state = FrameState::Deserialize( reader );

    driver.DrawFrame( state );
}

void RenderSceneCmd::RegisterCallHandler( SharedMemoryTaskQueue &task_queue )
{
    task_queue.RegisterTask(
        SharedMemoryTaskType::RENDER,
        std::make_unique<SharedMemoryTask>( RenderSceneTaskImpl ) );
}

} // namespace rh::rw::engine