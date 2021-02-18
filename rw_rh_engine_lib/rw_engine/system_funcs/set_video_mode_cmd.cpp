//
// Created by peter on 26.01.2021.
//

#include "set_video_mode_cmd.h"
#include <ipc/shared_memory_queue_client.h>
#include <render_driver/render_driver.h>

#include <DebugUtils/DebugLogger.h>
#include <Engine/Common/IDeviceState.h>
namespace rh::rw::engine
{
SetVideoModeIdCmdImpl::SetVideoModeIdCmdImpl(
    SharedMemoryTaskQueue &task_queue )
    : TaskQueue( task_queue )
{
}

bool SetVideoModeIdCmdImpl::Invoke( uint32_t id )
{
    TaskQueue.ExecuteTask( SharedMemoryTaskType::SET_CURRENT_VIDEO_MODE,
                           [&id]( MemoryWriter &&memory_writer ) {
                               // serialize
                               memory_writer.Write( &id );
                           } );
    return true;
}

void SetVideoModeIdTaskImpl( void *memory )
{
    assert( gRenderDriver );
    auto &driver = *gRenderDriver;

    // execute
    MemoryReader reader( memory );
    auto         id = *reader.Read<uint32_t>();
    if ( !driver.GetDeviceState().SetCurrentDisplayMode( id ) )
    {
        debug::DebugLogger::ErrorFmt( "Failed to select video mode #%i", id );
    }
}

void SetVideoModeIdCmdImpl::RegisterCallHandler(
    SharedMemoryTaskQueue &task_queue )
{
    task_queue.RegisterTask(
        SharedMemoryTaskType::SET_CURRENT_VIDEO_MODE,
        std::make_unique<SharedMemoryTask>( SetVideoModeIdTaskImpl ) );
}

} // namespace rh::rw::engine
