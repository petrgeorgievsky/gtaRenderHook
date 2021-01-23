//
// Created by peter on 20.01.2021.
//

#include "render_client.h"
#include <Engine/EngineConfigBlock.h>
namespace rh::rw::engine
{
RenderClient::RenderClient()
{
    /// initialize SM task queue
    TaskQueue =
        std::make_unique<SharedMemoryTaskQueue>( SharedMemoryTaskQueueInfo{
            .mName = "RenderHookTaskQueue",
            .mSize = 1024 * 1024 *
                     rh::engine::EngineConfigBlock::It.SharedMemorySizeMB,
            .mOwner = true } );

    /// Create render driver sub-process
    STARTUPINFOA start_info{ .cb = sizeof( start_info ) };
    CreateProcessA( IPCSettings::mProcessName.c_str(), nullptr, nullptr,
                    nullptr, false, 0, nullptr, nullptr, &start_info,
                    &RenderDriverProcess );
}

RenderClient::~RenderClient()
{
    TaskQueue->SendExitEvent();
    TaskQueue.reset();
    if ( RenderDriverProcess.hProcess )
        TerminateProcess( RenderDriverProcess.hProcess, 0 );
}
} // namespace rh::rw::engine