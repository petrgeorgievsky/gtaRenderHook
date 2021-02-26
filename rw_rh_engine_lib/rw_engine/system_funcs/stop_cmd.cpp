//
// Created by peter on 24.01.2021.
//

#include "stop_cmd.h"
#include <Engine/Common/IDeviceState.h>
#include <ipc/shared_memory_queue_client.h>
#include <render_client/imgui_state_recorder.h>
#include <render_driver/render_driver.h>

namespace rh::rw::engine
{

StopSystemCmdImpl::StopSystemCmdImpl( SharedMemoryTaskQueue &task_queue )
    : TaskQueue( task_queue )
{
}

bool StopSystemCmdImpl::Invoke()
{
    RemoveWinProcHook();
    TaskQueue.ExecuteTask( SharedMemoryTaskType::DESTROY_WINDOW );
    return true;
}

void DestroyWindowTaskImpl( void * )
{
    assert( gRenderDriver );
    auto &driver = *gRenderDriver;
    driver.CloseMainWindow();
}

void StopSystemCmdImpl::RegisterCallHandler( SharedMemoryTaskQueue &task_queue )
{
    task_queue.RegisterTask(
        SharedMemoryTaskType::DESTROY_WINDOW,
        std::make_unique<SharedMemoryTask>( DestroyWindowTaskImpl ) );
}

} // namespace rh::rw::engine
