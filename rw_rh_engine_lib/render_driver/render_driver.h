//
// Created by peter on 19.01.2021.
//

#pragma once
#include <common_headers.h>
#include <ipc/shared_memory_queue_client.h>
#include <memory>
#include <thread>

namespace rh::engine
{
class IDeviceState;
class IWindow;
} // namespace rh::engine

namespace rh::rw::engine
{
class EngineResourceHolder;
class FramebufferLoop;
class IFrameRenderer;

struct FrameState;
class RenderDriver
{
  public:
    RenderDriver();
    ~RenderDriver();

    void RegisterTasks();

    bool OpenMainWindow( HWND window );
    bool CloseMainWindow();

    void DrawFrame( const FrameState &frame_state );

    EngineResourceHolder &GetResources();

    // TODO: Remove after refactoring
    SharedMemoryTaskQueue &GetTaskQueue()
    {
        assert( TaskQueue );
        return *TaskQueue;
    }

    rh::engine::IDeviceState &GetDeviceState()
    {
        assert( DeviceState );
        return *DeviceState;
    }

    rh::engine::IWindow &GetMainWindow()
    {
        assert( MainWindow );
        return *MainWindow;
    }

  private:
    std::atomic<bool>                         IsRunning{ true };
    std::unique_ptr<SharedMemoryTaskQueue>    TaskQueue;
    std::unique_ptr<std::thread>              TaskQueueThread;
    std::unique_ptr<rh::engine::IDeviceState> DeviceState;
    std::unique_ptr<rh::engine::IWindow>      MainWindow;
    std::unique_ptr<EngineResourceHolder>     Resources;
    std::unique_ptr<IFrameRenderer>           Renderer;
    std::unique_ptr<FramebufferLoop>          FrameLoop;
};

/**
 * Global RenderDriver
 */
extern std::unique_ptr<RenderDriver> gRenderDriver;
} // namespace rh::rw::engine