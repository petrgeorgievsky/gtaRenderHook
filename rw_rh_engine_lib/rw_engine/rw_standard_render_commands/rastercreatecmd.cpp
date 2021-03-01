#include "rastercreatecmd.h"

#include <rw_engine/rh_backend/raster_backend.h>
#include <rw_engine/system_funcs/rw_device_system_globals.h>

#include <DebugUtils/DebugLogger.h>

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

    // Save just in case someone relies on it(e.g. wsfix)
    m_pRaster->originalWidth  = m_pRaster->width;
    m_pRaster->originalHeight = m_pRaster->height;

    /* Retrieve a pointer to internal raster */
    auto &internalRaster    = BackendRasterPlugin::GetData( m_pRaster );
    internalRaster.mImageId = BackendRasterPlugin::NullRasterId;

    if ( m_nFlags & rwRASTERDONTALLOCATE )
        return true;

    return true;
}
