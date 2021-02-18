//
// Created by peter on 25.01.2021.
//

#include "get_adapter_count_cmd.h"
#include "rw_device_system_globals.h"
#include <DebugUtils/DebugLogger.h>
#include <Engine/Common/IDeviceState.h>
#include <ipc/shared_memory_queue_client.h>
#include <render_driver/render_driver.h>

namespace rh::rw::engine
{
GetAdapterCountCmdImpl::GetAdapterCountCmdImpl(
    SharedMemoryTaskQueue &task_queue )
    : TaskQueue( task_queue )
{
}

uint32_t GetAdapterCountCmdImpl::Invoke()
{
    uint32_t count = 0;
    TaskQueue.ExecuteTask( SharedMemoryTaskType::GET_ADAPTER_COUNT,
                           EmptySerializer,
                           [&count]( MemoryReader &&memory_reader ) {
                               // deserialize
                               count = *memory_reader.Read<uint32_t>();
                           } );
    return count;
}

void GetAdapterCountTaskImpl( void *memory )
{
    assert( gRenderDriver );
    auto &driver = *gRenderDriver;

    MemoryWriter writer( memory );

    // execute
    uint32_t count = 0;
    if ( !driver.GetDeviceState().GetAdaptersCount( count ) )
    {
        debug::DebugLogger::Error( "Failed to retrieve adapter count" );
        count = 0;
    }
    writer.Write( &count );
}

void GetAdapterCountCmdImpl::RegisterCallHandler(
    SharedMemoryTaskQueue &task_queue )
{
    task_queue.RegisterTask(
        SharedMemoryTaskType::GET_ADAPTER_COUNT,
        std::make_unique<SharedMemoryTask>( GetAdapterCountTaskImpl ) );
}
} // namespace rh::rw::engine
