//
// Created by peter on 19.01.2021.
//
#include "start_cmd.h"
#include "rw_device_system_globals.h"
#include <Engine/Common/IDeviceState.h>
#include <ipc/shared_memory_queue_client.h>
#include <rendering_loop/ray_tracing/RayTracingRenderer.h>
namespace rh::rw::engine
{

bool StartSystemCmdImpl::Invoke( HWND window )
{
    mTaskQueue.ExecuteTask( SharedMemoryTaskType::CREATE_WINDOW,
                            [window]( MemoryWriter &&memory_writer ) {
                                // serialize
                                memory_writer.Write( &window );
                            } );
    return true;
}

void StartSystemCmdImpl::RegisterCallHandler()
{
    gRenderDriver->GetTaskQueue().RegisterTask(
        SharedMemoryTaskType::CREATE_WINDOW,
        std::make_unique<SharedMemoryTask>( []( void *memory ) {
            MemoryReader reader( memory );
            MemoryWriter writer( memory );

            // execute
            HWND wnd = *reader.Read<HWND>();
            gRenderDriver->OpenMainWindow( wnd );

            EngineState::gCameraState = std::make_unique<CameraState>(
                gRenderDriver->GetDeviceState() );
            EngineState::gFrameRenderer =
                std::make_shared<RayTracingRenderer>();
        } ) );
}
} // namespace rh::rw::engine