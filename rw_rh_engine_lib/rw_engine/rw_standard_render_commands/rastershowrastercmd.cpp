#include "rastershowrastercmd.h"
#include <render_client/render_client.h>
#include <rw_engine/system_funcs/render_scene_cmd.h>

using namespace rh::rw::engine;

RwRasterShowRasterCmd::RwRasterShowRasterCmd( RwRaster *raster, int32_t flags )
    : m_pRaster( raster ), m_nFlags( flags )
{
}

bool RwRasterShowRasterCmd::Execute()
{
    RenderSceneCmd cmd{ gRenderClient->GetTaskQueue() };
    cmd.Invoke();
    return true;
}
