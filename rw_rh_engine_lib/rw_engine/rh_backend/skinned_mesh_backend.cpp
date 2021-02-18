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

} // namespace rh::rw::engine