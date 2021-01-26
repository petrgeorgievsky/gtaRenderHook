//
// Created by peter on 26.01.2021.
//

#include "common_headers.h"

#include <cstdint>
#include <span>

namespace rh::engine
{
struct DisplayModeInfo;
}
namespace rh::rw::engine
{
class SharedMemoryTaskQueue;
class GetVideoModeInfoCmdImpl
{
  public:
    GetVideoModeInfoCmdImpl( SharedMemoryTaskQueue &task_queue );
    bool        Invoke( uint32_t id, rh::engine::DisplayModeInfo &info );
    static void RegisterCallHandler( SharedMemoryTaskQueue &task_queue );

  private:
    SharedMemoryTaskQueue &TaskQueue;
};
} // namespace rh::rw::engine