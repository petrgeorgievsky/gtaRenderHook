#include "rastershowrastercmd.h"
#include "../global_definitions.h"
#include <Engine/IRenderer.h>
using namespace rh::rw::engine;

RwRasterShowRasterCmd::RwRasterShowRasterCmd( RwRaster* raster, int32_t flags ):
    m_pRaster( raster ), m_nFlags( flags )
{

}

bool RwRasterShowRasterCmd::Execute()
{
    /* Retrieve a pointer to internal raster */
    void* internalRaster = GetInternalRaster( m_pRaster );

    return rh::engine::g_pRHRenderer->Present( internalRaster );
}
