//
// Created by peter on 24.04.2020.
//

#include "mesh_rendering_backend.h"
#include <Engine/Common/IBuffer.h>
#include <Engine/Common/IDeviceState.h>
#include <Engine/Common/IShader.h>
#include <Engine/VulkanImpl/VulkanCommandBuffer.h>
#include <Engine/VulkanImpl/VulkanDeviceState.h>
#include <rw_engine/system_funcs/rw_device_system_globals.h>
using namespace rh::rw::engine;

rh::engine::ResourcePool<BackendMeshData> *BackendMeshManager::SceneMeshData =
    nullptr;

uint64_t
rh::rw::engine::CreateBackendMesh( const BackendMeshInitData &initData )
{
    uint64_t result = 0xBADF00D;
    DeviceGlobals::SharedMemoryTaskQueue->ExecuteTask(
        SharedMemoryTaskType::MESH_LOAD,
        [&initData]( MemoryWriter &&memory_writer ) {
            // serialize
            memory_writer.Write( &initData.mVertexCount );
            memory_writer.Write( &initData.mIndexCount );

            memory_writer.Write( initData.mVertexData, initData.mVertexCount );
            memory_writer.Write( initData.mIndexData, initData.mIndexCount );

            uint32_t split_count = initData.mSplits.size();
            memory_writer.Write( &split_count );
            memory_writer.Write( initData.mSplits.data(), split_count );

            uint32_t mat_count = initData.mMaterials.size();
            memory_writer.Write( &mat_count );
            memory_writer.Write( initData.mMaterials.data(), mat_count );
        },
        [&result]( MemoryReader &&memory_reader ) {
            // deserialize
            result = *memory_reader.Read<uint64_t>();
        } );
    return result;
}

void rh::rw::engine::CreateBackendMeshImpl( void *memory )
{
    uint32_t memory_offset = sizeof( uint64_t );

    BackendMeshData     backendMeshData{};
    BackendMeshInitData initData{};
    initData.mVertexCount = *static_cast<uint64_t *>( memory );
    initData.mIndexCount  = *static_cast<uint64_t *>(
        static_cast<void *>( static_cast<char *>( memory ) + memory_offset ) );
    memory_offset += sizeof( uint64_t );
    initData.mVertexData = static_cast<VertexDescPosColorUVNormals *>(
        static_cast<void *>( static_cast<char *>( memory ) + memory_offset ) );
    memory_offset +=
        sizeof( decltype( initData.mVertexData[0] ) ) * initData.mVertexCount;
    initData.mIndexData = static_cast<uint16_t *>(
        static_cast<void *>( static_cast<char *>( memory ) + memory_offset ) );
    memory_offset +=
        sizeof( decltype( initData.mIndexData[0] ) ) * initData.mIndexCount;
    auto split_count = *static_cast<uint32_t *>(
        static_cast<void *>( static_cast<char *>( memory ) + memory_offset ) );
    memory_offset += sizeof( uint32_t );
    initData.mSplits.resize( split_count );
    CopyMemory( initData.mSplits.data(),
                static_cast<char *>( memory ) + memory_offset,
                sizeof( decltype( initData.mSplits[0] ) ) * split_count );
    memory_offset += sizeof( decltype( initData.mSplits[0] ) ) * split_count;
    auto material_count = *static_cast<uint32_t *>(
        static_cast<void *>( static_cast<char *>( memory ) + memory_offset ) );
    memory_offset += sizeof( uint32_t );
    initData.mMaterials.resize( material_count );
    CopyMemory( initData.mMaterials.data(),
                static_cast<char *>( memory ) + memory_offset,
                sizeof( decltype( initData.mMaterials[0] ) ) * material_count );

    rh::engine::BufferCreateInfo ib_info{};
    ib_info.mSize  = initData.mIndexCount * sizeof( int16_t );
    ib_info.mUsage = rh::engine::BufferUsage::IndexBuffer |
                     rh::engine::BufferUsage::StorageBuffer;
    ib_info.mInitDataPtr         = initData.mIndexData;
    backendMeshData.mIndexBuffer = new RefCountedBuffer(
        DeviceGlobals::RenderHookDevice->CreateBuffer( ib_info ) );

    rh::engine::BufferCreateInfo vb_info{};
    vb_info.mSize =
        initData.mVertexCount * sizeof( VertexDescPosColorUVNormals );
    vb_info.mUsage = rh::engine::BufferUsage::VertexBuffer |
                     rh::engine::BufferUsage::StorageBuffer;
    vb_info.mInitDataPtr          = initData.mVertexData;
    backendMeshData.mVertexBuffer = new RefCountedBuffer(
        DeviceGlobals::RenderHookDevice->CreateBuffer( vb_info ) );

    backendMeshData.mVertexCount = initData.mVertexCount;
    backendMeshData.mIndexCount  = initData.mIndexCount;
    backendMeshData.mSplits      = initData.mSplits;
    backendMeshData.mMaterials   = initData.mMaterials;

    auto id = BackendMeshManager::SceneMeshData->RequestResource(
        std::move( backendMeshData ) );

    *static_cast<uint64_t *>( memory ) = id;
}

void rh::rw::engine::DestroyBackendMesh( uint64_t id )
{
    DeviceGlobals::SharedMemoryTaskQueue->ExecuteTask(
        SharedMemoryTaskType::MESH_DELETE,
        [&id]( MemoryWriter &&memory_writer ) {
            // serialize
            memory_writer.Write( &id );
        },
        []( MemoryReader &&memory_reader ) {
            // deserialize
        } );
}

BackendRendererClient::BackendRendererClient()
{
    MeshData.resize( 10000 );
    DrawCallCount = 0;
    MaterialsData.resize( 20000 );
    MaterialCount = 0;
}

void BackendRendererClient::Flush()
{
    DrawCallCount = 0;
    MaterialCount = 0;
}

uint64_t BackendRendererClient::Serialize( MemoryWriter &memory_writer )
{
    /// Serialize schema:
    /// uint64 scene_materials_count
    /// GeometryMaterial scene_materials
    /// uint64 frame_draw_call_count
    /// Im2DDrawCall frame_drawcalls[frame_draw_call_count]

    // serialize drawcalls

    memory_writer.Write( &DrawCallCount );
    if ( DrawCallCount <= 0 )
        return memory_writer.Pos();

    memory_writer.Write( &MaterialCount );
    memory_writer.Write( MaterialsData.data(), MaterialCount );
    memory_writer.Write( MeshData.data(), DrawCallCount );

    return memory_writer.Pos();
}

void BackendRendererClient::RecordDrawCall(
    const DrawCallInfo &info, const std::vector<MaterialData> &materials )
{
    if ( MaterialsData.size() < MaterialCount + materials.size() )
        MaterialsData.resize( MaterialCount + materials.size() );

    MeshData[DrawCallCount]                    = info;
    MeshData[DrawCallCount].mMaterialListStart = MaterialCount;
    MeshData[DrawCallCount].mMaterialListCount = materials.size();
    DrawCallCount++;
    for ( auto i = 0; i < materials.size(); i++ )
        MaterialsData[MaterialCount + i] = materials[i];
    MaterialCount += materials.size();
}

uint64_t BackendRenderer::Render( void *                      memory,
                                  rh::engine::ICommandBuffer *cmd_buffer )
{
    uint64_t memory_offset   = 0;
    uint64_t draw_call_count = *static_cast<uint64_t *>(
        static_cast<void *>( static_cast<char *>( memory ) + memory_offset ) );

    memory_offset += sizeof( uint64_t );
    if ( draw_call_count <= 0 )
        return memory_offset;

    for ( uint64_t i = 0; i < draw_call_count; i++ )
    {
        auto *drawCall = static_cast<DrawCallInfo *>( static_cast<void *>(
            static_cast<char *>( memory ) + memory_offset ) );
        // auto &mesh     = BackendMeshManager::MeshData[drawCall->mMeshId];
        // mPipelines[drawCall->mPipelineId]->Draw( cmd_buffer, mesh );
        memory_offset += sizeof( DrawCallInfo );
    }
    return memory_offset;
}

void BackendRenderer::RegisterPipeline( RenderingPipeline *pipe, uint64_t id )
{
    mPipelines[id] = pipe;
}

void BackendRenderer::InitPipelines( FrameInfo *              frame,
                                     rh::engine::IRenderPass *main_render_pass )
{
    for ( const auto &item : mPipelines )
        item.second->Init( main_render_pass );
}

void BackendRenderer::DispatchSomeRays( FrameInfo *                 pInfo,
                                        rh::engine::ICommandBuffer *pBuffer )
{
}
rh::engine::IImageView *BackendRenderer::GetRaytraceView() { return nullptr; }

BackendRenderer::BackendRenderer() = default;
bool RefCountedBuffer::Release( RefCountedBuffer *buffer )
{
    buffer->mRefCount--;
    if ( buffer->mRefCount <= 0 )
    {
        delete buffer->mData;
    }
    return buffer->mRefCount <= 0;
}
