//
// Created by peter on 24.04.2020.
//

#include "mesh_rendering_backend.h"
#include <Engine/Common/IBuffer.h>
#include <Engine/Common/IDeviceState.h>
#include <Engine/Common/IShader.h>
#include <Engine/VulkanImpl/VulkanCommandBuffer.h>
#include <Engine/VulkanImpl/VulkanDeviceState.h>
#include <rw_engine/system_funcs/mesh_load_cmd.h>
#include <rw_engine/system_funcs/mesh_unload_cmd.h>
#include <rw_engine/system_funcs/rw_device_system_globals.h>
using namespace rh::rw::engine;

uint64_t
rh::rw::engine::CreateBackendMesh( const BackendMeshInitData &initData )
{
    return LoadMeshCmdImpl( gRenderClient->GetTaskQueue() ).Invoke( initData );
}

void rh::rw::engine::DestroyBackendMesh( uint64_t id )
{
    UnloadMeshCmdImpl cmd( gRenderClient->GetTaskQueue() );
    cmd.Invoke( id );
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

std::span<MaterialData>
BackendRendererClient::AllocateDrawCallMaterials( uint64_t count )
{
    if ( MaterialsData.size() < MaterialCount + count )
        MaterialsData.resize( MaterialCount + count );

    MeshData[DrawCallCount].mMaterialListStart = MaterialCount;
    MeshData[DrawCallCount].mMaterialListCount = count;
    return std::span<MaterialData>( &MaterialsData[MaterialCount], count );
}

void BackendRendererClient::RecordDrawCall( const DrawCallInfo &info )
{
    auto count              = MeshData[DrawCallCount].mMaterialListCount;
    MeshData[DrawCallCount] = info;
    MeshData[DrawCallCount].mMaterialListStart = MaterialCount;
    MeshData[DrawCallCount].mMaterialListCount = count;
    MaterialCount += MeshData[DrawCallCount].mMaterialListCount;
    DrawCallCount++;
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
