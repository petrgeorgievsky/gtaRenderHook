//
// Created by peter on 25.01.2021.
//

#pragma once
#include "common_headers.h"

#include <cstdint>

namespace rh::rw::engine
{
class SharedMemoryTaskQueue;
class GetAdapterCountCmdImpl
{
  public:
    GetAdapterCountCmdImpl( SharedMemoryTaskQueue &task_queue );
    uint32_t    Invoke();
    static void RegisterCallHandler( SharedMemoryTaskQueue &task_queue );

  private:
    SharedMemoryTaskQueue &TaskQueue;
};
} // namespace rh::rw::engine