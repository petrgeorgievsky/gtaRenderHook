#include "rasterunlockcmd.h"
#include <Engine/Common/types/image_buffer_format.h>
#include <TestUtils/test_dump_util.h>
#include <common_headers.h>
#include <rw_engine/rh_backend/raster_backend.h>
#include <rw_engine/rw_api_injectors.h>
#include <rw_engine/system_funcs/load_texture_cmd.h>
#include <rw_game_hooks.h>

using namespace rh::rw::engine;
RwRasterUnlockCmd::RwRasterUnlockCmd( RwRaster *raster ) : m_pRaster( raster )
{
}

bool RwRasterUnlockCmd::Execute()
{
    // TODO: Handle different lock/unlock patterns,
    // right now it just updates the texture on unlock
    if ( m_pRaster->privateFlags & 0x4 )
    {
        auto *internalRaster = GetBackendRasterExt( m_pRaster );

        LoadTextureCmdImpl load_texture_cmd( gRenderClient->GetTaskQueue() );
        RasterHeader       header{
            .mWidth  = static_cast<uint32_t>( m_pRaster->width ),
            .mHeight = static_cast<uint32_t>( m_pRaster->height ),
            .mDepth  = 1,
            .mFormat =
                static_cast<uint32_t>( rh::engine::ImageBufferFormat::BGRA8 ),
            .mMipLevelCount = 1 };
        
        internalRaster->mImageId = load_texture_cmd.Invoke(
            header, [this]( MemoryWriter &writer, MipLevelHeader &mip_header ) {
                mip_header.mSize   = m_pRaster->width * m_pRaster->height * 4;
                mip_header.mStride = m_pRaster->stride;
                writer.Skip( sizeof( MipLevelHeader ) );
                writer.Write( m_pRaster->cpPixels, mip_header.mSize );
                return true;
            } );
    }
    m_pRaster->privateFlags = 0;
    /* Restore the original width, height & stride */
    m_pRaster->width  = m_pRaster->originalWidth;
    m_pRaster->height = m_pRaster->originalHeight;

    m_pRaster->stride = 0;

    free( m_pRaster->cpPixels );
    m_pRaster->cpPixels = nullptr;
    return true;
}
