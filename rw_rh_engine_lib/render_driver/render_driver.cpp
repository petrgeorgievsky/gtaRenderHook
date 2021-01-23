//
// Created by peter on 19.01.2021.
//

#include "render_driver.h"
#include <Engine/Common/IDeviceState.h>
#include <Engine/Common/IImageBuffer.h>
#include <Engine/Common/IImageView.h>
#include <Engine/EngineConfigBlock.h>
#include <Engine/VulkanImpl/VulkanDeviceState.h>
namespace rh::rw::engine
{

RenderDriver::RenderDriver()
{
    DeviceState = std::make_unique<rh::engine::VulkanDeviceState>();

    /// initialize SM task queue
    TaskQueue =
        std::make_unique<SharedMemoryTaskQueue>( SharedMemoryTaskQueueInfo{
            .mName = "RenderHookTaskQueue",
            .mSize = 1024 * 1024 *
                     rh::engine::EngineConfigBlock::It.SharedMemorySizeMB,
            .mOwner =
                IPCSettings::mMode == IPCRenderMode::MultiThreadedRenderer } );

    TaskQueueThread = std::make_unique<std::jthread>( [this]() {
        while ( IsRunning )
            TaskQueue->TaskLoop();
    } );
    // DeviceState =
}

bool RenderDriver::OpenMainWindow( HWND window )
{
    unsigned int display_mode;

    if ( !DeviceState->GetCurrentDisplayMode( display_mode ) )
        return false;

    // initialize device state
    if ( !DeviceState->Init() )
        return false;
    MainWindow = std::unique_ptr<rh::engine::IWindow>(
        DeviceState->CreateDeviceWindow( window, { display_mode, true } ) );

    Resources = std::make_unique<EngineResourceHolder>();

    return MainWindow != nullptr;
}

bool RenderDriver::CloseMainWindow()
{
    DeviceState->WaitForGPU();

    Resources.reset();

    MainWindow.reset();

    return DeviceState->Shutdown();
}

EngineResourceHolder &RenderDriver::GetResources()
{
    assert( Resources );
    return *Resources;
}

RenderDriver::~RenderDriver() { IsRunning = false; }

void RasterData::Release()
{
    delete mImageView;
    delete mImageBuffer;
    mImageView   = nullptr;
    mImageBuffer = nullptr;
}

EngineResourceHolder::EngineResourceHolder()
    : RasterPool( 11000, []( RasterData &data, uint64_t ) { data.Release(); } ),
      SkinMeshPool( 1000,
                    []( SkinMeshData &data, uint64_t id ) {
                        if ( data.mIndexBuffer &&
                             RefCountedBuffer::Release( data.mIndexBuffer ) )
                            delete data.mIndexBuffer;
                        if ( data.mVertexBuffer &&
                             RefCountedBuffer::Release( data.mVertexBuffer ) )
                            delete data.mVertexBuffer;
                    } ),
      MeshPool( 10000, []( BackendMeshData &obj, uint64_t id ) {
          if ( obj.mIndexBuffer &&
               RefCountedBuffer::Release( obj.mIndexBuffer ) )
              delete obj.mIndexBuffer;
          if ( obj.mIndexBuffer &&
               RefCountedBuffer::Release( obj.mVertexBuffer ) )
              delete obj.mVertexBuffer;
      } )
{
}

void EngineResourceHolder::GC()
{
    SkinMeshPool.CollectGarbage( 1000 );
    MeshPool.CollectGarbage( 120 );
    // MaterialGlobals::SceneMaterialPool->CollectGarbage( 10000 );
    RasterPool.CollectGarbage( 100 );
}
} // namespace rh::rw::engine