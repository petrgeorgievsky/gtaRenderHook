//
// Created by peter on 19.01.2021.
//

#include "render_driver.h"
#include "framebuffer_loop.h"
#include "gpu_resources/resource_mgr.h"

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

#include <DebugUtils/DebugLogger.h>
#include <Engine/Common/IDeviceState.h>
#include <Engine/Common/IImageBuffer.h>
#include <Engine/Common/IImageView.h>
#include <Engine/EngineConfigBlock.h>
#include <Engine/VulkanImpl/VulkanDeviceState.h>

#include <rendering_loop/ray_tracing/RayTracingRenderer.h>

namespace rh::rw::engine
{
std::unique_ptr<RenderDriver> gRenderDriver = nullptr;

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

RenderDriver::~RenderDriver()
{
    IsRunning = false;
    if ( TaskQueueThread && TaskQueueThread->joinable() )
        TaskQueueThread->join();
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
    if ( MainWindow == nullptr )
        return false;

    Resources = std::make_unique<EngineResourceHolder>();

    if ( Resources == nullptr )
        return false;

    RendererCreateInfo renderer_info{ *DeviceState, *MainWindow, *Resources };
    Renderer = std::make_unique<RayTracingRenderer>( renderer_info );

    FramebufferLoopCreateInfo fb_info{ *DeviceState, *MainWindow, *Renderer };
    FrameLoop = std::make_unique<FramebufferLoop>( fb_info );

    return true;
}

bool RenderDriver::CloseMainWindow()
{
    DeviceState->WaitForGPU();

    FrameLoop.reset();

    Renderer.reset();

    Resources.reset();

    MainWindow.reset();

    return DeviceState->Shutdown();
}

void RenderDriver::DrawFrame( const FrameState &frame_state )
{
    FrameLoop->Run( frame_state );
    Resources->GC();
}

EngineResourceHolder &RenderDriver::GetResources()
{
    assert( Resources );
    return *Resources;
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

} // namespace rh::rw::engine