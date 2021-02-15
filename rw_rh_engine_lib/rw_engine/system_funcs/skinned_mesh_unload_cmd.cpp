//
// Created by peter on 12.02.2021.
//

#include "skinned_mesh_unload_cmd.h"
#include "rw_device_system_globals.h"
#include <ipc/shared_memory_queue_client.h>
#include <render_driver/render_driver.h>
#include <rw_engine/rh_backend/raster_backend.h>

namespace rh::rw::engine
{
SkinnedMeshUnloadCmdImpl::SkinnedMeshUnloadCmdImpl(
    SharedMemoryTaskQueue &task_queue )
    : TaskQueue( task_queue )
{
}

bool SkinnedMeshUnloadCmdImpl::Invoke( uint64_t id )
{
    TaskQueue.ExecuteTask( SharedMemoryTaskType::SKINNED_MESH_UNLOAD,
                           [&id]( MemoryWriter &&memory_writer ) {
                               // serialize
                               memory_writer.Write( &id );
                           } );
    return true;
}

void SkinnedMeshUnloadTaskImpl( void *memory )
{
    using namespace rh::engine;
    assert( gRenderDriver );
    auto &driver    = *gRenderDriver;
    auto &resources = driver.GetResources();
    auto &mesh_pool = resources.GetSkinMeshPool();

    MemoryReader reader( memory );
    mesh_pool.FreeResource( *reader.Read<uint64_t>() );
}

void SkinnedMeshUnloadCmdImpl::RegisterCallHandler(
    SharedMemoryTaskQueue &task_queue )
{
    task_queue.RegisterTask(
        SharedMemoryTaskType::SKINNED_MESH_UNLOAD,
        std::make_unique<SharedMemoryTask>( SkinnedMeshUnloadTaskImpl ) );
}
} // namespace rh::rw::engine