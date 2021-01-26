//
// Created by peter on 26.01.2021.
//
#pragma once
#include "common_headers.h"

#include <cstdint>
#include <span>

namespace rh::rw::engine
{
class SharedMemoryTaskQueue;
class SetVideoModeIdCmdImpl
{
  public:
    SetVideoModeIdCmdImpl( SharedMemoryTaskQueue &task_queue );
    bool        Invoke( uint32_t id );
    static void RegisterCallHandler( SharedMemoryTaskQueue &task_queue );

  private:
    SharedMemoryTaskQueue &TaskQueue;
};
} // namespace rh::rw::engine