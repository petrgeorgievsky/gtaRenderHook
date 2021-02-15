#include "rastershowrastercmd.h"
#include "../global_definitions.h"
#include "../rh_backend/camera_backend.h"
#include "../rh_backend/raster_backend.h"
#include <Engine/Common/ISwapchain.h>
#include <Engine/Common/IWindow.h>
#include <render_client/render_client.h>
#include <render_loop.h>
using namespace rh::rw::engine;

RwRasterShowRasterCmd::RwRasterShowRasterCmd( RwRaster *raster, int32_t flags )
    : m_pRaster( raster ), m_nFlags( flags )
{
}

bool RwRasterShowRasterCmd::Execute()
{
    /* Retrieve a pointer to internal raster */
    ExecuteRender();
    GetCurrentSceneGraph()->mFrameInfo.camera_updated = false;
    GetCurrentSceneGraph()->mFrameInfo.mLightCount    = 0;
    return true;
}
