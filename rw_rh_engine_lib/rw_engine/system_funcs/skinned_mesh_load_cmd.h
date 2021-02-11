//
// Created by peter on 12.02.2021.
//

#pragma once
#include "common_headers.h"

#include <cstdint>
#include <functional>
#include <span>

namespace rh::rw::engine
{
class SharedMemoryTaskQueue;
struct SkinnedMeshInitData;
class SkinnedMeshLoadCmdImpl
{
  public:
    SkinnedMeshLoadCmdImpl( SharedMemoryTaskQueue &task_queue );
    uint64_t    Invoke( const SkinnedMeshInitData &init_data );
    static void RegisterCallHandler( SharedMemoryTaskQueue &task_queue );

  private:
    SharedMemoryTaskQueue &TaskQueue;
};
} // namespace rh::rw::engine