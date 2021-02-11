//
// Created by peter on 10.02.2021.
//
#pragma once
#include "common_headers.h"

#include <cstdint>
#include <functional>
#include <span>

namespace rh::rw::engine
{
class SharedMemoryTaskQueue;
struct RasterHeader;
class MemoryWriter;
struct MipLevelHeader;
class RasterLoadCmdImpl
{
    using WriteMipLevelFunc =
        std::function<bool( MemoryWriter &, MipLevelHeader & )>;

  public:
    RasterLoadCmdImpl( SharedMemoryTaskQueue &task_queue );
    int64_t     Invoke( const RasterHeader &header,
                        WriteMipLevelFunc   write_mip_level );
    static void RegisterCallHandler( SharedMemoryTaskQueue &task_queue );

  private:
    SharedMemoryTaskQueue &TaskQueue;
};
} // namespace rh::rw::engine