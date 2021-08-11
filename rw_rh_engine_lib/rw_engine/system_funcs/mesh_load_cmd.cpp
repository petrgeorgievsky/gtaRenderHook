//
// Created by peter on 11.02.2021.
//

#include "mesh_load_cmd.h"
#include "rw_device_system_globals.h"
#include <Engine/Common/IDeviceState.h>
#include <ipc/shared_memory_queue_client.h>
#include <render_driver/gpu_resources/resource_mgr.h>
#include <render_driver/render_driver.h>
#include <rw_engine/rh_backend/mesh_rendering_backend.h>
#include <rw_engine/rh_backend/raster_backend.h>

namespace rh::rw::engine
{
LoadMeshCmdImpl::LoadMeshCmdImpl( SharedMemoryTaskQueue &task_queue )
    : TaskQueue( task_queue )
{
}

uint64_t LoadMeshCmdImpl::Invoke( const BackendMeshInitData &mesh_data )
{
    uint64_t result = 0xBADF00D;
    TaskQueue.ExecuteTask(
        SharedMemoryTaskType::MESH_LOAD,
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

            uint32_t mat_count = mesh_data.mMaterials.size();
            memory_writer.Write( &mat_count );
            memory_writer.Write( mesh_data.mMaterials.data(), mat_count );
        },
        [&result]( MemoryReader &&memory_reader ) {
            // deserialize
            result = *memory_reader.Read<uint64_t>();
        } );
    return result;
}

void LoadMeshTaskImpl( void *memory )
{
    using namespace rh::engine;
    assert( gRenderDriver );
    auto &driver    = *gRenderDriver;
    auto &device    = driver.GetDeviceState();
    auto &resources = driver.GetResources();
    auto &mesh_pool = resources.GetMeshPool();

    uint64_t     mesh_id = 0;
    MemoryWriter writer( memory );
    MemoryReader reader( memory );

    BackendMeshInitData init_data{};
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

    auto material_count = *reader.Read<uint32_t>();
    auto materials      = std::span<GeometryMaterial>(
        reader.Read<GeometryMaterial>( material_count ), material_count );

    init_data.mMaterials.reserve( material_count );
    std::ranges::copy( materials, std::back_inserter( init_data.mMaterials ) );

    BufferCreateInfo ib_info{};
    ib_info.mSize  = init_data.mIndexCount * sizeof( uint16_t );
    ib_info.mUsage = BufferUsage::IndexBuffer | BufferUsage::StorageBuffer;
    ib_info.mInitDataPtr = init_data.mIndexData;

    BufferCreateInfo vb_info{};
    vb_info.mSize =
        init_data.mVertexCount * sizeof( VertexDescPosColorUVNormals );
    vb_info.mUsage = BufferUsage::VertexBuffer | BufferUsage::StorageBuffer;
    vb_info.mInitDataPtr = init_data.mVertexData;

    BackendMeshData backend_mesh_data{};
    backend_mesh_data.mIndexBuffer =
        new RefCountedBuffer( device.CreateBuffer( ib_info ) );
    backend_mesh_data.mVertexBuffer =
        new RefCountedBuffer( device.CreateBuffer( vb_info ) );

    backend_mesh_data.mVertexCount = init_data.mVertexCount;
    backend_mesh_data.mIndexCount  = init_data.mIndexCount;
    backend_mesh_data.mSplits      = std::move( init_data.mSplits );
    backend_mesh_data.mMaterials   = std::move( init_data.mMaterials );

    backend_mesh_data.EmissiveTriangles.reserve( init_data.mIndexCount / 3 );
    for ( auto tri_id = 0; tri_id < init_data.mIndexCount / 3; tri_id++ )
    {
        auto idx_a = init_data.mIndexData[tri_id * 3 + 0],
             idx_b = init_data.mIndexData[tri_id * 3 + 1],
             idx_c = init_data.mIndexData[tri_id * 3 + 2];

        PackedLight tri_light{};
        tri_light.Triangle.V0[0] = init_data.mVertexData[idx_a].x;
        tri_light.Triangle.V0[1] = init_data.mVertexData[idx_a].y;
        tri_light.Triangle.V0[2] = init_data.mVertexData[idx_a].z;

        tri_light.Triangle.V1[0] = init_data.mVertexData[idx_b].x;
        tri_light.Triangle.V1[1] = init_data.mVertexData[idx_b].y;
        tri_light.Triangle.V1[2] = init_data.mVertexData[idx_b].z;

        tri_light.Triangle.V2[0] = init_data.mVertexData[idx_c].x;
        tri_light.Triangle.V2[1] = init_data.mVertexData[idx_c].y;
        tri_light.Triangle.V2[2] = init_data.mVertexData[idx_c].z;

        tri_light.Triangle.Intensity = init_data.mVertexData[idx_a].emissive;
        //
        if ( init_data.mVertexData[idx_a].emissive > 0 )
            backend_mesh_data.EmissiveTriangles.push_back( tri_light );
    }

    mesh_id = mesh_pool.RequestResource( std::move( backend_mesh_data ) );

    writer.Write( &mesh_id );
}

void LoadMeshCmdImpl::RegisterCallHandler( SharedMemoryTaskQueue &task_queue )
{
    task_queue.RegisterTask(
        SharedMemoryTaskType::MESH_LOAD,
        std::make_unique<SharedMemoryTask>( LoadMeshTaskImpl ) );
}
} // namespace rh::rw::engine