//
// Created by peter on 17.04.2020.
//
#include "ipc_utils.h"

#include <Engine/Common/IDeviceState.h>
#include <Engine/EngineConfigBlock.h>
#include <Engine/VulkanImpl/VulkanDeviceState.h>
#include <rw_engine/system_funcs/rw_device_system_globals.h>

namespace rh::rw::engine
{

static PROCESS_INFORMATION child_proc_info{};

void InitClient()
{
    /// initialize SM task queue
    DeviceGlobals::SharedMemoryTaskQueue = new SharedMemoryTaskQueue(
        { .mName = "RenderHookTaskQueue",
          .mSize = 1024 * 1024 *
                   rh::engine::EngineConfigBlock::It.SharedMemorySizeMB,
          .mOwner = true } );
    /// Create sub-process
    STARTUPINFOA start_info{ .cb = sizeof( start_info ) };
    CreateProcess( IPCSettings::mProcessName.c_str(), nullptr, nullptr, nullptr,
                   false, 0, nullptr, nullptr, &start_info, &child_proc_info );
}
void ShutdownClient()
{
    DeviceGlobals::SharedMemoryTaskQueue->SendExitEvent();
    delete DeviceGlobals::SharedMemoryTaskQueue;
    if ( child_proc_info.hProcess )
        TerminateProcess( child_proc_info.hProcess, 0 );
    child_proc_info = {};
}

void InitRenderer()
{
    DeviceGlobals::RenderHookDevice =
        std::make_unique<rh::engine::VulkanDeviceState>();
    /// initialize SM task queue
    DeviceGlobals::SharedMemoryTaskQueue = new SharedMemoryTaskQueue(
        { .mName = "RenderHookTaskQueue",
          .mSize = 1024 * 1024 *
                   rh::engine::EngineConfigBlock::It.SharedMemorySizeMB,
          .mOwner =
              IPCSettings::mMode == IPCRenderMode::MultiThreadedRenderer } );

    DeviceGlobals::SharedMemoryTaskQueueThread =
        std::make_unique<std::thread>( []() {
            while ( !DeviceGlobals::RenderThreadShallDie )
                DeviceGlobals::SharedMemoryTaskQueue->TaskLoop();
        } );
}

void ShutdownRenderer()
{
    DeviceGlobals::RenderThreadShallDie = true;
    DeviceGlobals::SharedMemoryTaskQueueThread->join();
    DeviceGlobals::RenderHookDevice.reset();
}
} // namespace rh::rw::engine