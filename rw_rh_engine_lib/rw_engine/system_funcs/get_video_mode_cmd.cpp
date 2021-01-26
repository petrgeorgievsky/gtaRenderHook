//
// Created by peter on 26.01.2021.
//

#include "get_video_mode_cmd.h"
#include "rw_device_system_globals.h"
#include <Engine/Common/IDeviceState.h>
#include <ipc/shared_memory_queue_client.h>

namespace rh::rw::engine
{
GetVideoModeIdCmdImpl::GetVideoModeIdCmdImpl(
    SharedMemoryTaskQueue &task_queue )
    : TaskQueue( task_queue )
{
}

uint32_t GetVideoModeIdCmdImpl::Invoke()
{

    uint32_t count = 0;
    TaskQueue.ExecuteTask( SharedMemoryTaskType::GET_CURRENT_VIDEO_MODE,
                           EmptySerializer,
                           [&count]( MemoryReader &&memory_reader ) {
                               // deserialize
                               count = *memory_reader.Read<uint32_t>();
                           } );
    return count;
}

void GetVideoModeIdTaskImpl( void *memory )
{
    assert( gRenderDriver );
    auto &driver = *gRenderDriver;
    // execute

    uint32_t     id = 0;
    MemoryWriter writer( memory );

    if ( !driver.GetDeviceState().GetCurrentDisplayMode( id ) )
    {
        debug::DebugLogger::Error(
            "Failed to retrieve current display video mode!" );
    }
    writer.Write( &id );
}

void GetVideoModeIdCmdImpl::RegisterCallHandler(
    SharedMemoryTaskQueue &task_queue )
{
    task_queue.RegisterTask(
        SharedMemoryTaskType::GET_CURRENT_VIDEO_MODE,
        std::make_unique<SharedMemoryTask>( GetVideoModeIdTaskImpl ) );
}

} // namespace rh::rw::engine