//
// Created by peter on 12.02.2021.
//

#include "skinned_mesh_load_cmd.h"
#include "rw_device_system_globals.h"
#include <Engine/Common/IDeviceState.h>
#include <ipc/shared_memory_queue_client.h>
#include <render_driver/render_driver.h>
#include <rw_engine/rh_backend/raster_backend.h>

namespace rh::rw::engine
{
SkinnedMeshLoadCmdImpl::SkinnedMeshLoadCmdImpl(
    SharedMemoryTaskQueue &task_queue )
    : TaskQueue( task_queue )
{
}

uint64_t SkinnedMeshLoadCmdImpl::Invoke( const SkinnedMeshInitData &mesh_data )
{
    uint64_t result = 0xBADF00D;
    TaskQueue.ExecuteTask(
        SharedMemoryTaskType::SKINNED_MESH_LOAD,
        [&mesh_data]( MemoryWriter &&memory_writer ) {
            // serialize
            memory_writer.Write( &mesh_data.mVertexCount );
            memory_writer.Write( &mesh_data.mIndexCount );

            memory_writer.Write( mesh_data.mVertexData,
                                 mesh_data.mVertexCount );
            memory_writer.Write( mesh_data.mIndexData, mesh_data.mIndexCount );

            uint32_t split_count = mesh_data.mSplits.size();
            memory_writer.Write( &split_count );
            memory_writer.Write( mesh_data.mSplits.data(), split_count );
        },
        [&result]( MemoryReader &&memory_reader ) {
            // deserialize
            result = *memory_reader.Read<uint64_t>();
        } );
    return result;
}

void SkinnedMeshLoadTaskImpl( void *memory )
{
    using namespace rh::engine;
    assert( gRenderDriver );
    auto &driver    = *gRenderDriver;
    auto &device    = driver.GetDeviceState();
    auto &resources = driver.GetResources();
    auto &mesh_pool = resources.GetSkinMeshPool();

    uint64_t     mesh_id = 0;
    MemoryWriter writer( memory );
    MemoryReader reader( memory );

    SkinnedMeshInitData init_data{};
    init_data.mVertexCount = *reader.Read<uint64_t>();
    init_data.mIndexCount  = *reader.Read<uint64_t>();
    init_data.mVertexData =
        reader.Read<VertexDescPosColorUVNormals>( init_data.mVertexCount );
    init_data.mIndexData = reader.Read<uint16_t>( init_data.mIndexCount );

    auto split_count = *reader.Read<uint32_t>();
    auto splits      = std::span<GeometrySplit>(
        reader.Read<GeometrySplit>( split_count ), split_count );

    init_data.mSplits.reserve( split_count );
    std::ranges::copy( splits, std::back_inserter( init_data.mSplits ) );

    BufferCreateInfo ib_info{};
    ib_info.mSize  = init_data.mIndexCount * sizeof( uint16_t );
    ib_info.mUsage = BufferUsage::IndexBuffer | BufferUsage::StorageBuffer;
    ib_info.mInitDataPtr = init_data.mIndexData;

    BufferCreateInfo vb_info{};
    vb_info.mSize =
        init_data.mVertexCount * sizeof( VertexDescPosColorUVNormals );
    vb_info.mUsage = BufferUsage::VertexBuffer | BufferUsage::StorageBuffer;
    vb_info.mInitDataPtr = init_data.mVertexData;

    SkinMeshData backend_mesh_data{};
    backend_mesh_data.mIndexBuffer =
        new RefCountedBuffer( device.CreateBuffer( ib_info ) );
    backend_mesh_data.mVertexBuffer =
        new RefCountedBuffer( device.CreateBuffer( vb_info ) );

    backend_mesh_data.mVertexCount = init_data.mVertexCount;
    backend_mesh_data.mIndexCount  = init_data.mIndexCount;

    mesh_id = mesh_pool.RequestResource( backend_mesh_data );

    writer.Write( &mesh_id );
}

void SkinnedMeshLoadCmdImpl::RegisterCallHandler(
    SharedMemoryTaskQueue &task_queue )
{
    task_queue.RegisterTask(
        SharedMemoryTaskType::SKINNED_MESH_LOAD,
        std::make_unique<SharedMemoryTask>( SkinnedMeshLoadTaskImpl ) );
}
} // namespace rh::rw::engine