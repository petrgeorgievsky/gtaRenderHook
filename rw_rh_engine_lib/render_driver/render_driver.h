//
// Created by peter on 19.01.2021.
//

#pragma once
#include <Engine/ResourcePool.h>
#include <common_headers.h>
#include <ipc/shared_memory_queue_client.h>
#include <memory>
#include <render_loop.h>
#include <thread>

namespace rh::engine
{
class IDeviceState;
class IWindow;
class IImageBuffer;
class IImageView;
} // namespace rh::engine

namespace rh::rw::engine
{
struct RasterData
{
    rh::engine::IImageBuffer *mImageBuffer;
    rh::engine::IImageView *  mImageView;
    void                      Release();
};

class EngineResourceHolder
{
  public:
    EngineResourceHolder();

    void GC();

    rh::engine::ResourcePool<RasterData> &GetRasterPool() { return RasterPool; }
    rh::engine::ResourcePool<SkinMeshData> &GetSkinMeshPool()
    {
        return SkinMeshPool;
    }
    rh::engine::ResourcePool<BackendMeshData> &GetMeshPool()
    {
        return MeshPool;
    }

  private:
    // Resources
    rh::engine::ResourcePool<RasterData>      RasterPool;
    rh::engine::ResourcePool<SkinMeshData>    SkinMeshPool;
    rh::engine::ResourcePool<BackendMeshData> MeshPool;
};

class RenderDriver
{
  public:
    RenderDriver();
    ~RenderDriver();

    void RegisterTasks();

    bool OpenMainWindow( HWND window );
    bool CloseMainWindow();

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
};

/**
 * Global RenderDriver
 */
extern std::unique_ptr<RenderDriver> gRenderDriver;
} // namespace rh::rw::engine