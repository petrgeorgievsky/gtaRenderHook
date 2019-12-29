#include "rastercreatecmd.h"
#include "../global_definitions.h"
#include "../rw_rh_convert_funcs.h"
#include <Engine/Common/types/image_buffer_info.h>
#include <Engine/Common/types/image_buffer_type.h>
#include <Engine/IRenderer.h>
using namespace rh::rw::engine;

RwRasterCreateCmd::RwRasterCreateCmd( RwRaster *raster, uint32_t flags )
    : m_pRaster( raster )
    , m_nFlags( flags )
{}

bool RwRasterCreateCmd::Execute()
{
    rh::engine::ImageBufferType imageBufferType = rh::engine::ImageBufferType::Unknown;

    /* Initialise structure to something sensible */
    m_pRaster->cType = static_cast<RwUInt8>( m_nFlags & rwRASTERTYPEMASK );
    m_pRaster->cFlags = static_cast<RwUInt8>( m_nFlags & 0xF0 );
    m_pRaster->cFormat = static_cast<RwUInt8>( ( m_nFlags & rwRASTERFORMATMASK ) >> 8 );
    m_pRaster->cpPixels = nullptr;
    m_pRaster->palette = nullptr;

    auto format = static_cast<RwRasterFormat>( m_nFlags & rwRASTERFORMATMASK );

    if ( m_pRaster->cType & rwRASTERTYPECAMERA )
        imageBufferType = rh::engine::ImageBufferType::BackBuffer;

    if ( m_pRaster->cType & rwRASTERTYPEZBUFFER )
        imageBufferType = rh::engine::ImageBufferType::DepthBuffer;

    if ( m_pRaster->cType & rwRASTERTYPETEXTURE )
        imageBufferType = rh::engine::ImageBufferType::TextureBuffer;

    /* Retrieve a pointer to internal raster */
    void *&internalRaster = GetInternalRaster( m_pRaster );

    internalRaster = nullptr;

    if ( m_nFlags & rwRASTERDONTALLOCATE )
        return true;
    rh::engine::ImageBufferInfo info{};
    info.width = static_cast<uint32_t>( m_pRaster->width );
    info.height = static_cast<uint32_t>( m_pRaster->height );
    info.type = imageBufferType;
    info.format = RwFormatToRHImageBufferFormat( format );
    internalRaster = rh::engine::g_pRHRenderer->AllocateImageBuffer( info );

    return internalRaster != nullptr;
}
