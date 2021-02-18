//
// Created by peter on 25.01.2021.
//

#include "set_adapter_cmd.h"

#include <ipc/shared_memory_queue_client.h>
#include <render_driver/render_driver.h>

#include <DebugUtils/DebugLogger.h>
#include <Engine/Common/IDeviceState.h>

namespace rh::rw::engine
{
SetAdapterIdCmdImpl::SetAdapterIdCmdImpl( SharedMemoryTaskQueue &task_queue )
    : TaskQueue( task_queue )
{
}
bool SetAdapterIdCmdImpl::Invoke( uint32_t id )
{
    TaskQueue.ExecuteTask( SharedMemoryTaskType::SET_CURRENT_ADAPTER,
                           [&id]( MemoryWriter &&memory_writer ) {
                               // serialize
                               memory_writer.Write( &id );
                           } );
    return true;
}

void SetAdapterIdTaskImpl( void *memory )
{
    assert( gRenderDriver );
    auto &driver = *gRenderDriver;

    // execute
    MemoryReader reader( memory );
    auto         id = *reader.Read<uint32_t>();
    if ( !driver.GetDeviceState().SetCurrentAdapter( id ) )
    {
        debug::DebugLogger::ErrorFmt( "Failed to select adapter #%i", id );
    }
}

void SetAdapterIdCmdImpl::RegisterCallHandler(
    SharedMemoryTaskQueue &task_queue )
{
    task_queue.RegisterTask(
        SharedMemoryTaskType::SET_CURRENT_ADAPTER,
        std::make_unique<SharedMemoryTask>( SetAdapterIdTaskImpl ) );
}

} // namespace rh::rw::engine
