//
// Created by peter on 13.05.2020.
//

#include "skinned_mesh_backend.h"
#include <Engine/Common/IBuffer.h>
#include <Engine/Common/IDeviceState.h>
#include <ipc/MemoryReader.h>
#include <ipc/MemoryWriter.h>
#include <rw_engine/system_funcs/rw_device_system_globals.h>
namespace rh::rw::engine
{

uint64_t CreateSkinMesh( const SkinnedMeshInitData &initData )
{
    uint64_t result = 0xBADF00D;
    gRenderClient->GetTaskQueue().ExecuteTask(
        SharedMemoryTaskType::SKINNED_MESH_LOAD,
        [&initData]( MemoryWriter &&writer ) {
            writer.Write( &initData.mVertexCount );
            writer.Write( &initData.mIndexCount );
            writer.Write( initData.mVertexData, initData.mVertexCount );
            writer.Write( initData.mIndexData, initData.mIndexCount );
            uint32_t split_count = initData.mSplits.size();
            writer.Write( &split_count );
            writer.Write( initData.mSplits.data(), split_count );
        },
        [&result]( MemoryReader &&memory_reader ) {
            result = *memory_reader.Read<uint64_t>();
        } );
    return result;
}

void CreateSkinMeshImpl( void *memory )
{
    auto &       resources      = gRenderDriver->GetResources();
    auto &       skin_mesh_pool = resources.GetSkinMeshPool();
    MemoryReader reader( memory );

    SkinMeshData        backendMeshData{};
    SkinnedMeshInitData initData{};
    initData.mVertexCount = *reader.Read<uint64_t>();
    initData.mIndexCount  = *reader.Read<uint64_t>();
    initData.mVertexData =
        reader.Read<VertexDescPosColorUVNormals>( initData.mVertexCount );
    initData.mIndexData = reader.Read<uint16_t>( initData.mIndexCount );

    auto split_count = *reader.Read<uint32_t>();
    initData.mSplits.resize( split_count );
    CopyMemory( initData.mSplits.data(),
                reader.Read<GeometrySplit>( split_count ),
                sizeof( GeometrySplit ) * split_count );

    rh::engine::BufferCreateInfo ib_info{};
    ib_info.mSize  = initData.mIndexCount * sizeof( int16_t );
    ib_info.mUsage = rh::engine::BufferUsage::IndexBuffer |
                     rh::engine::BufferUsage::StorageBuffer;
    ib_info.mInitDataPtr         = initData.mIndexData;
    backendMeshData.mIndexBuffer = new RefCountedBuffer(
        gRenderDriver->GetDeviceState().CreateBuffer( ib_info ) );

    rh::engine::BufferCreateInfo vb_info{};
    vb_info.mSize =
        initData.mVertexCount * sizeof( VertexDescPosColorUVNormals );
    vb_info.mUsage = rh::engine::BufferUsage::VertexBuffer |
                     rh::engine::BufferUsage::StorageBuffer;
    vb_info.mInitDataPtr          = initData.mVertexData;
    backendMeshData.mVertexBuffer = new RefCountedBuffer(
        gRenderDriver->GetDeviceState().CreateBuffer( vb_info ) );

    backendMeshData.mVertexCount = initData.mVertexCount;
    backendMeshData.mIndexCount  = initData.mIndexCount;

    auto id = skin_mesh_pool.RequestResource( backendMeshData );

    *static_cast<uint64_t *>( memory ) = id;
}

void DestroySkinMesh( uint64_t id )
{
    gRenderClient->GetTaskQueue().ExecuteTask(
        SharedMemoryTaskType::SKINNED_MESH_UNLOAD,
        [&id]( MemoryWriter &&writer ) {
            // serialize
            writer.Write( &id );
        },
        []( MemoryReader &&memory_reader ) {
            // deserialize
        } );
}

void DestroySkinMeshImpl( void *memory )
{
    auto &resources      = gRenderDriver->GetResources();
    auto &skin_mesh_pool = resources.GetSkinMeshPool();
    skin_mesh_pool.FreeResource( *static_cast<uint64_t *>( memory ) );
}

SkinRendererClient::SkinRendererClient()
{
    MeshData.resize( 100 );
    MaterialsData.resize( 2000 );
    DrawCallCount = 0;
    MaterialCount = 0;
}

std::span<MaterialData>
SkinRendererClient::AllocateDrawCallMaterials( uint64_t count )
{
    if ( MaterialsData.size() < MaterialCount + count )
        MaterialsData.resize( MaterialCount + count );

    MeshData[DrawCallCount].mMaterialListStart = MaterialCount;
    MeshData[DrawCallCount].mMaterialListCount = count;

    return std::span<MaterialData>( &MaterialsData[MaterialCount], count );
}

void SkinRendererClient::RecordDrawCall( const SkinDrawCallInfo &info )
{
    auto mat_count          = MeshData[DrawCallCount].mMaterialListCount;
    MeshData[DrawCallCount] = info;
    MeshData[DrawCallCount].mMaterialListStart = MaterialCount;
    MeshData[DrawCallCount].mMaterialListCount = mat_count;
    MaterialCount += mat_count;
    DrawCallCount++;
}

uint64_t SkinRendererClient::Serialize( MemoryWriter &writer )
{
    /// Serialize schema:
    /// uint64 skip_offset
    /// uint64 frame_draw_call_count
    /// uint64 material_count
    /// uint64 materials
    /// Im2DDrawCall frame_drawcalls[frame_draw_call_count]

    // serialize drawcalls
    auto &skip_offset = writer.Current<uint64_t>();
    writer.Skip( sizeof( uint64_t ) );
    writer.Write( &DrawCallCount );

    if ( DrawCallCount <= 0 )
    {
        skip_offset = writer.Pos();
        return writer.Pos();
    }
    writer.Write( &MaterialCount );
    writer.Write( MaterialsData.data(), MaterialCount );
    writer.Write( MeshData.data(), DrawCallCount );

    skip_offset = writer.Pos();
    return writer.Pos();
}

void SkinRendererClient::Flush()
{
    DrawCallCount = 0;
    MaterialCount = 0;
}

} // namespace rh::rw::engine