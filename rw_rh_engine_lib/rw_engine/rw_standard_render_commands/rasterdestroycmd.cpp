#include "rasterdestroycmd.h"
#include "../global_definitions.h"
#include <Engine/Common/types/image_buffer_type.h>
#include <Engine/IRenderer.h>

using namespace rh::rw::engine;
RwRasterDestroyCmd::RwRasterDestroyCmd( RwRaster *raster )
    : m_pRaster( raster )
{}

bool RwRasterDestroyCmd::Execute()
{
    if ( m_pRaster == nullptr )
        return true;
    /* Retrieve a pointer to internal raster */
    void *internalRaster = GetInternalRaster( m_pRaster );

    rh::engine::ImageBufferType imageBufferType = rh::engine::ImageBufferType::Unknown;

    if ( m_pRaster->cType & rwRASTERTYPECAMERA )
        imageBufferType = rh::engine::ImageBufferType::BackBuffer;
    if ( m_pRaster->cType & rwRASTERTYPEZBUFFER )
        imageBufferType = rh::engine::ImageBufferType::DepthBuffer;
    if ( m_pRaster->cType & rwRASTERTYPETEXTURE )
        imageBufferType = rh::engine::ImageBufferType::TextureBuffer;

    return rh::engine::g_pRHRenderer->FreeImageBuffer( internalRaster, imageBufferType );
}
