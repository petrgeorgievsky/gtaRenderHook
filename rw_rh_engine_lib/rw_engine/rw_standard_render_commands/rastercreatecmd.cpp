#include "rastercreatecmd.h"
#include "../global_definitions.h"
#include "../rh_backend/raster_backend.h"
#include "../system_funcs/rw_device_system_globals.h"
#include <DebugUtils/DebugLogger.h>
#include <Engine/Common/types/image_buffer_info.h>
#include <Engine/Common/types/image_buffer_type.h>

using namespace rh::rw::engine;

RwRasterCreateCmd::RwRasterCreateCmd( RwRaster *raster, uint32_t flags )
    : m_pRaster( raster ), m_nFlags( flags )
{
    rh::debug::DebugLogger::Log(
        std::to_string( reinterpret_cast<uint32_t>( raster ) ) +
            " RasterCreateCmd created...",
        rh::debug::LogLevel::ConstrDestrInfo );
}

bool RwRasterCreateCmd::Execute()
{
    /* Initialise structure to something sensible */
    m_pRaster->cType  = static_cast<uint8_t>( m_nFlags & rwRASTERTYPEMASK );
    m_pRaster->cFlags = static_cast<uint8_t>( m_nFlags & 0xF0 );
    m_pRaster->cFormat =
        static_cast<uint8_t>( ( m_nFlags & rwRASTERFORMATMASK ) >> 8 );
    m_pRaster->cpPixels = nullptr;
    m_pRaster->palette  = nullptr;

    /* Retrieve a pointer to internal raster */
    auto *internalRaster     = GetBackendRasterExt( m_pRaster );
    internalRaster->mImageId = gNullRasterId;

    if ( m_nFlags & rwRASTERDONTALLOCATE )
        return true;

    return true;
}
