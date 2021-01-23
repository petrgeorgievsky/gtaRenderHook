//
// Created by peter on 20.01.2021.
//

#pragma once
#include <common_headers.h>
#include <ipc/shared_memory_queue_client.h>
namespace rh::rw::engine
{
/*
class RwDeviceState
{
  public:
    RwDeviceState( RwDevice &device );

  private:
    RwDevice &       Device;
    RwSystemFunc     OldSystemFunc;
    RwStandardFunc * DeviceStandardFuncs;
    PluginPtrTable   PluginFuncs;
    ResourcePtrTable ResourceFuncs;
    SkinPtrTable     SkinFuncs;
};*/

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

  private:
    std::unique_ptr<SharedMemoryTaskQueue> TaskQueue;
    PROCESS_INFORMATION                    RenderDriverProcess{};
};
} // namespace rh::rw::engine