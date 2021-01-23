//
// Created by peter on 19.01.2021.
//
#pragma once
#include "common_headers.h"

#include <cstdint>

namespace rh::rw::engine
{
class SharedMemoryTaskQueue;
class StartSystemCmdImpl
{
  public:
    bool        Invoke( HWND window );
    static void RegisterCallHandler();

  private:
    SharedMemoryTaskQueue &mTaskQueue;
};
} // namespace rh::rw::engine