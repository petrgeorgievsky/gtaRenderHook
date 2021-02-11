//
// Created by peter on 10.02.2021.
//

#pragma once
#include "common_headers.h"

#include <cstdint>
#include <span>

namespace rh::rw::engine
{
class SharedMemoryTaskQueue;
class RasterDestroyCmdImpl
{
  public:
    RasterDestroyCmdImpl( SharedMemoryTaskQueue &task_queue );
    bool        Invoke( uint64_t raster_id );
    static void RegisterCallHandler( SharedMemoryTaskQueue &task_queue );

  private:
    SharedMemoryTaskQueue &TaskQueue;
};
} // namespace rh::rw::engine