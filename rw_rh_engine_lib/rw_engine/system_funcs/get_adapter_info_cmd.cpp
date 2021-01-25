//
// Created by peter on 25.01.2021.
//

#include "get_adapter_info_cmd.h"
#include "rw_device_system_globals.h"
#include <Engine/Common/IDeviceState.h>
#include <ipc/shared_memory_queue_client.h>
namespace rh::rw::engine
{

GetAdapterInfoCmdImpl::GetAdapterInfoCmdImpl(
    SharedMemoryTaskQueue &task_queue )
    : TaskQueue( task_queue )
{
}

bool GetAdapterInfoCmdImpl::Invoke( uint32_t id, std::span<char, 80> &info )
{
    TaskQueue.ExecuteTask(
        SharedMemoryTaskType::GET_ADAPTER_INFO,
        [&id]( MemoryWriter &&memory_writer ) {
            // serialize
            memory_writer.Write( &id );
        },
        [&info]( MemoryReader &&memory_reader ) {
            // deserialize
            const auto *     desc    = memory_reader.Read<char>( 80 );
            std::string_view desc_sv = std::string_view( desc, 80 );
            desc_sv.copy( info.data(), 80 );
        } );
    return true;
}

void GetAdapterInfoTaskImpl( void *memory )
{
    assert( gRenderDriver );
    auto &driver = *gRenderDriver;
    // execute
    std::array<char, 80> result{};
    MemoryReader         reader( memory );
    MemoryWriter         writer( memory );
    auto                 id = *reader.Read<int32_t>();

    std::string str;
    if ( !driver.GetDeviceState().GetAdapterInfo(
             static_cast<unsigned int>( id ), str ) )
    {
        debug::DebugLogger::ErrorFmt( "Failed to retrieve adapter #%n info",
                                      id );
    }
    result[str.copy( result.data(), result.size() - 1 )] = '\0';

    writer.Write( result.data(), result.size() );
}

void GetAdapterInfoCmdImpl::RegisterCallHandler(
    SharedMemoryTaskQueue &task_queue )
{
    task_queue.RegisterTask(
        SharedMemoryTaskType::GET_ADAPTER_INFO,
        std::make_unique<SharedMemoryTask>( GetAdapterInfoTaskImpl ) );
}

} // namespace rh::rw::engine