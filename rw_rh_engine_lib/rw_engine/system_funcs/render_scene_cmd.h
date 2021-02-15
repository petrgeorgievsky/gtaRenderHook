//
// Created by peter on 12.02.2021.
//

#pragma once
#include "common_headers.h"

#include <cstdint>
#include <span>

namespace rh::rw::engine
{
class SharedMemoryTaskQueue;
class RenderSceneCmd
{
  public:
    RenderSceneCmd( SharedMemoryTaskQueue &task_queue );
    bool        Invoke();
    static void RegisterCallHandler( SharedMemoryTaskQueue &task_queue );

  private:
    SharedMemoryTaskQueue &TaskQueue;
};
} // namespace rh::rw::engine