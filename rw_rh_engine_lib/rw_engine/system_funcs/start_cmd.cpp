//
// Created by peter on 19.01.2021.
//
#include "start_cmd.h"
#include "rw_device_system_globals.h"
#include <Engine/Common/IDeviceState.h>
#include <ipc/shared_memory_queue_client.h>
#include <render_driver/render_driver.h>
#include <rendering_loop/ray_tracing/RayTracingRenderer.h>

namespace rh::rw::engine
{

StartSystemCmdImpl::StartSystemCmdImpl( SharedMemoryTaskQueue &task_queue )
    : TaskQueue( task_queue )
{
}

bool StartSystemCmdImpl::Invoke( HWND window )
{
    TaskQueue.ExecuteTask( SharedMemoryTaskType::CREATE_WINDOW,
                           [window]( MemoryWriter &&memory_writer ) {
                               // serialize
                               memory_writer.Write( &window );
                           } );
    return true;
}

void CreateWindowTaskImpl( void *memory )
{
    MemoryReader reader( memory );
    MemoryWriter writer( memory );

    // execute
    HWND wnd = *reader.Read<HWND>();
    if ( !gRenderDriver->OpenMainWindow( wnd ) )
    {
        debug::DebugLogger::ErrorFmt( "Failed to open window:%X", wnd );
        return;
    }

    // Init renderer state
    // TODO: Refactor
    EngineState::gCameraState =
        std::make_unique<CameraState>( gRenderDriver->GetDeviceState() );
    EngineState::gFrameRenderer = std::make_shared<RayTracingRenderer>();
}

void StartSystemCmdImpl::RegisterCallHandler(
    SharedMemoryTaskQueue &task_queue )
{
    task_queue.RegisterTask(
        SharedMemoryTaskType::CREATE_WINDOW,
        std::make_unique<SharedMemoryTask>( CreateWindowTaskImpl ) );
}

} // namespace rh::rw::engine