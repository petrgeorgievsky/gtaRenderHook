//
// Created by peter on 11.02.2021.
//

#include "mesh_unload_cmd.h"
#include "rw_device_system_globals.h"
#include <ipc/shared_memory_queue_client.h>
#include <render_driver/gpu_resources/resource_mgr.h>
#include <render_driver/render_driver.h>
#include <rw_engine/rh_backend/raster_backend.h>

namespace rh::rw::engine
{
UnloadMeshCmdImpl::UnloadMeshCmdImpl( SharedMemoryTaskQueue &task_queue )
    : TaskQueue( task_queue )
{
}

bool UnloadMeshCmdImpl::Invoke( uint64_t id )
{
    TaskQueue.ExecuteTask( SharedMemoryTaskType::MESH_UNLOAD,
                           [&id]( MemoryWriter &&memory_writer ) {
                               // serialize
                               memory_writer.Write( &id );
                           } );
    return true;
}

void UnloadMeshTaskImpl( void *memory )
{
    using namespace rh::engine;
    assert( gRenderDriver );
    auto &driver    = *gRenderDriver;
    auto &resources = driver.GetResources();
    auto &mesh_pool = resources.GetMeshPool();

    MemoryReader reader( memory );
    mesh_pool.FreeResource( *reader.Read<uint64_t>() );
}

void UnloadMeshCmdImpl::RegisterCallHandler( SharedMemoryTaskQueue &task_queue )
{
    task_queue.RegisterTask(
        SharedMemoryTaskType::MESH_UNLOAD,
        std::make_unique<SharedMemoryTask>( UnloadMeshTaskImpl ) );
}
} // namespace rh::rw::engine