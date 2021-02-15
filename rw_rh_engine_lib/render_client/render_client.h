//
// Created by peter on 20.01.2021.
//

#pragma once
#include "client_render_state.h"
#include <common_headers.h>
#include <ipc/shared_memory_queue_client.h>
namespace rh::rw::engine
{

class RenderClient
{
  public:
    RenderClient();
    ~RenderClient();

    // TODO: Remove after refactoring
    SharedMemoryTaskQueue &GetTaskQueue()
    {
        assert( TaskQueue );
        return *TaskQueue;
    }

    ClientRenderState RenderState{};

  private:
    std::unique_ptr<SharedMemoryTaskQueue> TaskQueue;
    PROCESS_INFORMATION                    RenderDriverProcess{};
};
} // namespace rh::rw::engine