//
// Created by peter on 26.01.2021.
//

#include "get_video_mode_info_cmd.h"
#include "rw_device_system_globals.h"
#include <Engine/Common/IDeviceState.h>
#include <ipc/shared_memory_queue_client.h>

namespace rh::rw::engine
{

GetVideoModeInfoCmdImpl::GetVideoModeInfoCmdImpl(
    SharedMemoryTaskQueue &task_queue )
    : TaskQueue( task_queue )
{
}

bool GetVideoModeInfoCmdImpl::Invoke( uint32_t                     id,
                                      rh::engine::DisplayModeInfo &info )
{
    TaskQueue.ExecuteTask(
        SharedMemoryTaskType::GET_VIDEO_MODE_INFO,
        [&id]( MemoryWriter &&memory_writer ) {
            // serialize
            memory_writer.Write( &id );
        },
        [&info]( MemoryReader &&memory_reader ) {
            // deserialize
            info = *memory_reader.Read<rh::engine::DisplayModeInfo>();
        } );
    return true;
}

void GetVideoModeInfoTaskImpl( void *memory )
{
    assert( gRenderDriver );
    auto &driver = *gRenderDriver;
    // execute

    MemoryReader reader( memory );
    MemoryWriter writer( memory );
    uint32_t     id = *reader.Read<uint32_t>();

    rh::engine::DisplayModeInfo display_mode{};
    if ( !driver.GetDeviceState().GetDisplayModeInfo( id, display_mode ) )
    {
        debug::DebugLogger::ErrorFmt(
            "Failed to retrieve display video mode #%i info!", id );
    }
    writer.Write( &display_mode );
}

void GetVideoModeInfoCmdImpl::RegisterCallHandler(
    SharedMemoryTaskQueue &task_queue )
{
    task_queue.RegisterTask(
        SharedMemoryTaskType::GET_VIDEO_MODE_INFO,
        std::make_unique<SharedMemoryTask>( GetVideoModeInfoTaskImpl ) );
}

} // namespace rh::rw::engine