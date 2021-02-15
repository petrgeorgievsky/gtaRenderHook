#include "rasterunlockcmd.h"
#include <Engine/Common/types/image_buffer_format.h>
#include <TestUtils/test_dump_util.h>
#include <common_headers.h>
#include <render_client/render_client.h>
#include <rw_engine/rh_backend/raster_backend.h>
#include <rw_engine/rw_api_injectors.h>
#include <rw_engine/system_funcs/raster_load_cmd.h>
#include <rw_engine/system_funcs/raster_unload_cmd.h>
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
        auto &internalRaster = BackendRasterPlugin::GetData( m_pRaster );

        auto old_image = internalRaster.mImageId;
        assert( gRenderClient );
        auto &client = *gRenderClient;

        RasterLoadCmdImpl load_texture_cmd( client.GetTaskQueue() );
        RasterHeader      header{
            .mWidth  = static_cast<uint32_t>( m_pRaster->width ),
            .mHeight = static_cast<uint32_t>( m_pRaster->height ),
            .mDepth  = 1,
            .mFormat =
                static_cast<uint32_t>( rh::engine::ImageBufferFormat::BGRA8 ),
            .mMipLevelCount = 1 };

        internalRaster.mImageId = load_texture_cmd.Invoke(
            header, [this]( MemoryWriter &writer, MipLevelHeader &mip_header ) {
                mip_header.mSize   = m_pRaster->width * m_pRaster->height * 4;
                mip_header.mStride = m_pRaster->stride;
                writer.Skip( sizeof( MipLevelHeader ) );
                writer.Write( m_pRaster->cpPixels, mip_header.mSize );
                return true;
            } );

        // Destroy old raster if there was one
        if ( old_image != BackendRasterPlugin::NullRasterId )
        {
            RasterDestroyCmdImpl cmd( client.GetTaskQueue() );
            cmd.Invoke( old_image );
        }
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
