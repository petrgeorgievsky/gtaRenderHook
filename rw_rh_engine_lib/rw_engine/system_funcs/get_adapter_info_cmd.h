//
// Created by peter on 25.01.2021.
//

#pragma once
#include "common_headers.h"

#include <cstdint>
#include <span>

namespace rh::rw::engine
{
class SharedMemoryTaskQueue;
class GetAdapterInfoCmdImpl
{
  public:
    GetAdapterInfoCmdImpl( SharedMemoryTaskQueue &task_queue );
    bool        Invoke( uint32_t id, std::span<char, 80> &info );
    static void RegisterCallHandler( SharedMemoryTaskQueue &task_queue );

  private:
    SharedMemoryTaskQueue &TaskQueue;
};
} // namespace rh::rw::engine