//
// Created by peter on 24.04.2020.
//

#include "mesh_rendering_backend.h"
#include <Engine/Common/IBuffer.h>
#include <Engine/Common/IDeviceState.h>
#include <Engine/Common/IShader.h>
#include <render_client/render_client.h>
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

bool RefCountedBuffer::Release( RefCountedBuffer *buffer )
{
    buffer->mRefCount--;
    if ( buffer->mRefCount <= 0 )
    {
        delete buffer->mData;
    }
    return buffer->mRefCount <= 0;
}
