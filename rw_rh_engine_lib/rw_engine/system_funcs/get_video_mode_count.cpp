//
// Created by peter on 26.01.2021.
//

#include "get_video_mode_count.h"
#include "rw_device_system_globals.h"
#include <DebugUtils/DebugLogger.h>
#include <Engine/Common/IDeviceState.h>
#include <ipc/shared_memory_queue_client.h>
#include <render_driver/render_driver.h>

namespace rh::rw::engine
{
GetVideoModeCountCmdImpl::GetVideoModeCountCmdImpl(
    SharedMemoryTaskQueue &task_queue )
    : TaskQueue( task_queue )
{
}

uint32_t GetVideoModeCountCmdImpl::Invoke()
{

    uint32_t count = 0;
    TaskQueue.ExecuteTask( SharedMemoryTaskType::GET_VIDEO_MODE_COUNT,
                           EmptySerializer,
                           [&count]( MemoryReader &&memory_reader ) {
                               // deserialize
                               count = *memory_reader.Read<uint32_t>();
                           } );
    return count;
}

void GetVideoModeCountTaskImpl( void *memory )
{
    assert( gRenderDriver );
    auto &driver = *gRenderDriver;
    // execute

    uint32_t     count = 0;
    MemoryWriter writer( memory );

    if ( !driver.GetDeviceState().GetDisplayModeCount( 0, count ) )
    {
        debug::DebugLogger::Error(
            "Failed to retrieve current display video mode count!" );
    }
    writer.Write( &count );
}

void GetVideoModeCountCmdImpl::RegisterCallHandler(
    SharedMemoryTaskQueue &task_queue )
{
    task_queue.RegisterTask(
        SharedMemoryTaskType::GET_VIDEO_MODE_COUNT,
        std::make_unique<SharedMemoryTask>( GetVideoModeCountTaskImpl ) );
}

} // namespace rh::rw::engine
