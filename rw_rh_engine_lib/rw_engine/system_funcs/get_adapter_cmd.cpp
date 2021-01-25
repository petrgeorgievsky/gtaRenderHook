//
// Created by peter on 25.01.2021.
//

#include "get_adapter_cmd.h"
#include "rw_device_system_globals.h"
#include <Engine/Common/IDeviceState.h>
#include <ipc/shared_memory_queue_client.h>

namespace rh::rw::engine
{
GetAdapterIdCmdImpl::GetAdapterIdCmdImpl( SharedMemoryTaskQueue &task_queue )
    : TaskQueue( task_queue )
{
}

uint32_t GetAdapterIdCmdImpl::Invoke()
{
    uint32_t id = 0;
    TaskQueue.ExecuteTask( SharedMemoryTaskType::GET_CURRENT_ADAPTER,
                           EmptySerializer,
                           [&id]( MemoryReader &&memory_reader ) {
                               // deserialize
                               id = *memory_reader.Read<uint32_t>();
                           } );
    return id;
}

void GetAdapterIdTaskImpl( void *memory )
{
    assert( gRenderDriver );
    auto &driver = *gRenderDriver;

    // execute
    MemoryWriter writer( memory );

    unsigned int adapter = 0;
    if ( !driver.GetDeviceState().GetCurrentAdapter( adapter ) )
    {
        debug::DebugLogger::Error( "Failed to retrieve selected adapter id" );
        adapter = 0;
    }
    auto adapter_id = static_cast<uint32_t>( adapter );
    writer.Write( &adapter_id );
}

void GetAdapterIdCmdImpl::RegisterCallHandler(
    SharedMemoryTaskQueue &task_queue )
{
    task_queue.RegisterTask(
        SharedMemoryTaskType::GET_CURRENT_ADAPTER,
        std::make_unique<SharedMemoryTask>( GetAdapterIdTaskImpl ) );
}

} // namespace rh::rw::engine
