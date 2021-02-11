//
// Created by peter on 11.02.2021.
//

#pragma once
#include "common_headers.h"

#include <cstdint>
#include <functional>
#include <span>

namespace rh::rw::engine
{
class SharedMemoryTaskQueue;
struct BackendMeshInitData;
class UnloadMeshCmdImpl
{
  public:
    UnloadMeshCmdImpl( SharedMemoryTaskQueue &task_queue );
    bool        Invoke( uint64_t mesh_id );
    static void RegisterCallHandler( SharedMemoryTaskQueue &task_queue );

  private:
    SharedMemoryTaskQueue &TaskQueue;
};
} // namespace rh::rw::engine