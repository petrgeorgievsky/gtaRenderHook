//
// Created by peter on 13.05.2020.
//

#include "skinned_mesh_backend.h"
#include <Engine/Common/IBuffer.h>
#include <Engine/Common/IDeviceState.h>
#include <ipc/MemoryReader.h>
#include <ipc/MemoryWriter.h>
#include <render_client/render_client.h>
#include <rw_engine/system_funcs/rw_device_system_globals.h>
#include <rw_engine/system_funcs/skinned_mesh_load_cmd.h>
#include <rw_engine/system_funcs/skinned_mesh_unload_cmd.h>
namespace rh::rw::engine
{

uint64_t CreateSkinMesh( const SkinnedMeshInitData &initData )
{
    SkinnedMeshLoadCmdImpl cmd( gRenderClient->GetTaskQueue() );

    return cmd.Invoke( initData );
}

void DestroySkinMesh( uint64_t id )
{
    SkinnedMeshUnloadCmdImpl cmd( gRenderClient->GetTaskQueue() );

    cmd.Invoke( id );
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
    writer.Write( &DrawCallCount );

    if ( DrawCallCount <= 0 )
    {
        return writer.Pos();
    }
    writer.Write( &MaterialCount );
    writer.Write( MaterialsData.data(), MaterialCount );
    writer.Write( MeshData.data(), DrawCallCount );

    return writer.Pos();
}

void SkinRendererClient::Flush()
{
    DrawCallCount = 0;
    MaterialCount = 0;
}

} // namespace rh::rw::engine