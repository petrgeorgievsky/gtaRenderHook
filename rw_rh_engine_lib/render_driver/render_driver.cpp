//
// Created by peter on 19.01.2021.
//

#include "render_driver.h"
#include <Engine/Common/IDeviceState.h>
#include <Engine/Common/IImageBuffer.h>
#include <Engine/Common/IImageView.h>
#include <Engine/EngineConfigBlock.h>
#include <Engine/VulkanImpl/VulkanDeviceState.h>

#include <rw_engine/system_funcs/get_adapter_cmd.h>
#include <rw_engine/system_funcs/get_adapter_count_cmd.h>
#include <rw_engine/system_funcs/get_adapter_info_cmd.h>
#include <rw_engine/system_funcs/get_video_mode_cmd.h>
#include <rw_engine/system_funcs/get_video_mode_count.h>
#include <rw_engine/system_funcs/get_video_mode_info_cmd.h>
#include <rw_engine/system_funcs/mesh_load_cmd.h>
#include <rw_engine/system_funcs/mesh_unload_cmd.h>
#include <rw_engine/system_funcs/raster_load_cmd.h>
#include <rw_engine/system_funcs/raster_lock_cmd.h>
#include <rw_engine/system_funcs/raster_unload_cmd.h>
#include <rw_engine/system_funcs/render_scene_cmd.h>
#include <rw_engine/system_funcs/set_adapter_cmd.h>
#include <rw_engine/system_funcs/set_video_mode_cmd.h>
#include <rw_engine/system_funcs/skinned_mesh_load_cmd.h>
#include <rw_engine/system_funcs/skinned_mesh_unload_cmd.h>
#include <rw_engine/system_funcs/start_cmd.h>
#include <rw_engine/system_funcs/stop_cmd.h>

namespace rh::rw::engine
{

RenderDriver::RenderDriver()
{
    DeviceState = std::make_unique<rh::engine::VulkanDeviceState>();

    /// initialize SM task queue
    TaskQueue =
        std::make_unique<SharedMemoryTaskQueue>( SharedMemoryTaskQueueInfo{
            .mName = "RenderHookTaskQueue",
            .mSize = 1024 * 1024 *
                     rh::engine::EngineConfigBlock::It.SharedMemorySizeMB,
            .mOwner =
                IPCSettings::mMode == IPCRenderMode::MultiThreadedRenderer } );

    TaskQueueThread = std::make_unique<std::thread>( [this]() {
        while ( IsRunning )
            TaskQueue->TaskLoop();
    } );
}

bool RenderDriver::OpenMainWindow( HWND window )
{
    unsigned int display_mode;

    if ( !DeviceState->GetCurrentDisplayMode( display_mode ) )
    {
        debug::DebugLogger::Error( "Failed to retrieve current display mode" );
        return false;
    }

    // initialize device state
    if ( !DeviceState->Init() )
    {
        debug::DebugLogger::Error( "Failed to initialize device state" );
        return false;
    }
    MainWindow = std::unique_ptr<rh::engine::IWindow>(
        DeviceState->CreateDeviceWindow( window, { display_mode, true } ) );

    Resources = std::make_unique<EngineResourceHolder>();

    return MainWindow != nullptr;
}

bool RenderDriver::CloseMainWindow()
{
    DeviceState->WaitForGPU();

    Resources.reset();

    MainWindow.reset();

    return DeviceState->Shutdown();
}

EngineResourceHolder &RenderDriver::GetResources()
{
    assert( Resources );
    return *Resources;
}

RenderDriver::~RenderDriver()
{
    IsRunning = false;
    if ( TaskQueueThread && TaskQueueThread->joinable() )
        TaskQueueThread->join();
}

void RenderDriver::RegisterTasks()
{
    /// Register driver tasks
    assert( TaskQueue );
    auto &task_queue = *TaskQueue;

    StartSystemCmdImpl::RegisterCallHandler( task_queue );
    StopSystemCmdImpl::RegisterCallHandler( task_queue );

    // Adapter datasource
    GetAdapterCountCmdImpl::RegisterCallHandler( task_queue );
    GetAdapterIdCmdImpl::RegisterCallHandler( task_queue );
    SetAdapterIdCmdImpl::RegisterCallHandler( task_queue );
    GetAdapterInfoCmdImpl::RegisterCallHandler( task_queue );

    // VideoMode datasource
    GetVideoModeCountCmdImpl::RegisterCallHandler( task_queue );
    GetVideoModeIdCmdImpl::RegisterCallHandler( task_queue );
    SetVideoModeIdCmdImpl::RegisterCallHandler( task_queue );
    GetVideoModeInfoCmdImpl::RegisterCallHandler( task_queue );

    RasterDestroyCmdImpl::RegisterCallHandler( task_queue );
    RasterLoadCmdImpl::RegisterCallHandler( task_queue );
    RasterLockCmdImpl::RegisterCallHandler( task_queue );

    LoadMeshCmdImpl::RegisterCallHandler( task_queue );
    UnloadMeshCmdImpl::RegisterCallHandler( task_queue );
    SkinnedMeshLoadCmdImpl::RegisterCallHandler( task_queue );
    SkinnedMeshUnloadCmdImpl::RegisterCallHandler( task_queue );

    RenderSceneCmd::RegisterCallHandler( task_queue );
}

void RasterData::Release()
{
    delete mImageView;
    delete mImageBuffer;
    mImageView   = nullptr;
    mImageBuffer = nullptr;
}

EngineResourceHolder::EngineResourceHolder()
    : RasterPool( 11000, []( RasterData &data, uint64_t ) { data.Release(); } ),
      SkinMeshPool( 1000,
                    []( SkinMeshData &data, uint64_t id ) {
                        if ( data.mIndexBuffer &&
                             RefCountedBuffer::Release( data.mIndexBuffer ) )
                            delete data.mIndexBuffer;
                        if ( data.mVertexBuffer &&
                             RefCountedBuffer::Release( data.mVertexBuffer ) )
                            delete data.mVertexBuffer;
                    } ),
      MeshPool( 10000, []( BackendMeshData &obj, uint64_t id ) {
          if ( obj.mIndexBuffer &&
               RefCountedBuffer::Release( obj.mIndexBuffer ) )
              delete obj.mIndexBuffer;
          if ( obj.mIndexBuffer &&
               RefCountedBuffer::Release( obj.mVertexBuffer ) )
              delete obj.mVertexBuffer;
      } )
{
}

void EngineResourceHolder::GC()
{
    SkinMeshPool.CollectGarbage( 1000 );
    MeshPool.CollectGarbage( 120 );
    RasterPool.CollectGarbage( 100 );
}
} // namespace rh::rw::engine