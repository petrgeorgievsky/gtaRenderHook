//
// Created by peter on 10.02.2021.
//

#include "raster_destroy_cmd.h"
#include "rw_device_system_globals.h"
#include <Engine/Common/IDeviceState.h>
#include <ipc/shared_memory_queue_client.h>

namespace rh::rw::engine
{

RasterDestroyCmdImpl::RasterDestroyCmdImpl( SharedMemoryTaskQueue &task_queue )
    : TaskQueue( task_queue )
{
}

bool RasterDestroyCmdImpl::Invoke( uint64_t raster_id )
{
    TaskQueue.ExecuteTask( SharedMemoryTaskType::DESTROY_RASTER,
                           [&raster_id]( MemoryWriter &&memory_writer ) {
                               // serialize
                               memory_writer.Write( &raster_id );
                           } );
    return true;
}

void RasterDestroyCmdImpl::RegisterCallHandler(
    SharedMemoryTaskQueue &task_queue )
{
    task_queue.RegisterTask(
        SharedMemoryTaskType::DESTROY_RASTER,
        std::make_unique<SharedMemoryTask>( []( void *memory ) {
            auto &resources   = gRenderDriver->GetResources();
            auto &raster_pool = resources.GetRasterPool();
            // execute
            raster_pool.FreeResource( *static_cast<uint64_t *>( memory ) );
        } ) );
}

} // namespace rh::rw::engine