//
// Created by peter on 24.01.2021.
//
#pragma once
#include "common_headers.h"

#include <cstdint>

namespace rh::rw::engine
{
class SharedMemoryTaskQueue;
class StopSystemCmdImpl
{
  public:
    StopSystemCmdImpl( SharedMemoryTaskQueue &task_queue );
    bool        Invoke();
    static void RegisterCallHandler( SharedMemoryTaskQueue &task_queue );

  private:
    SharedMemoryTaskQueue &TaskQueue;
};
} // namespace rh::rw::engine