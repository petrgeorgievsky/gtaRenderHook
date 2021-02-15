#include "rasterdestroycmd.h"
#include "../global_definitions.h"
#include <Engine/Common/types/image_buffer_type.h>
#include <Engine/IRenderer.h>
#include <ipc/shared_memory_queue_client.h>
#include <rw_engine/rh_backend/raster_backend.h>
#include <rw_engine/system_funcs/rw_device_system_globals.h>

using namespace rh::rw::engine;
RwRasterDestroyCmd::RwRasterDestroyCmd( RwRaster *raster ) : m_pRaster( raster )
{
}

bool RwRasterDestroyCmd::Execute()
{
    if ( m_pRaster == nullptr )
        return true;
    /* Retrieve a pointer to internal raster */
    auto *rasExt = GetBackendRasterExt( m_pRaster );
    if ( rasExt->mImageId == gNullRasterId )
        return true;

    return true; // rh::engine::g_pRHRenderer->FreeImageBuffer( internalRaster,
                 // imageBufferType );
}
