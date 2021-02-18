//
// Created by peter on 20.01.2021.
//

#include "render_client.h"
#include <Engine/EngineConfigBlock.h>
#include <rw_engine/rh_backend/material_backend.h>
#include <rw_engine/rh_backend/raster_backend.h>

namespace rh::rw::engine
{
std::unique_ptr<RenderClient> gRenderClient = nullptr;

/**
 * RenderClient RenderWare plugins storage
 */
class ClientPlugins
{
  public:
    explicit ClientPlugins( const PluginPtrTable &plugin_cb )
        : Raster( plugin_cb ), Material( plugin_cb )
    {
    }

  private:
    BackendRasterPlugin   Raster;
    BackendMaterialPlugin Material;
};

RenderClient::RenderClient()
{
    /// initialize SM task queue
    TaskQueue =
        std::make_unique<SharedMemoryTaskQueue>( SharedMemoryTaskQueueInfo{
            .mName = "RenderHookTaskQueue",
            .mSize = 1024 * 1024 *
                     rh::engine::EngineConfigBlock::It.SharedMemorySizeMB,
            .mOwner = true } );

    /// Create render driver sub-process
    STARTUPINFOA start_info{ .cb = sizeof( start_info ) };
    CreateProcessA( IPCSettings::mProcessName.c_str(), nullptr, nullptr,
                    nullptr, false, 0, nullptr, nullptr, &start_info,
                    &RenderDriverProcess );
}

RenderClient::~RenderClient()
{
    TaskQueue->SendExitEvent();
    TaskQueue.reset();
    if ( RenderDriverProcess.hProcess )
        TerminateProcess( RenderDriverProcess.hProcess, 0 );
}

bool RenderClient::RegisterPlugins( const PluginPtrTable &plugin_cb )
{
    Plugins = std::make_unique<ClientPlugins>( plugin_cb );
    return true;
}

} // namespace rh::rw::engine