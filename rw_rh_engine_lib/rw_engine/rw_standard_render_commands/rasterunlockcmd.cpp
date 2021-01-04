#include "rasterunlockcmd.h"
#include <Engine/Common/types/image_buffer_format.h>
#include <common_headers.h>
#include <rw_engine/rh_backend/raster_backend.h>
#include <rw_engine/rw_api_injectors.h>
#include <rw_game_hooks.h>

using namespace rh::rw::engine;
RwRasterUnlockCmd::RwRasterUnlockCmd( RwRaster *raster ) : m_pRaster( raster )
{
}

bool RwRasterUnlockCmd::Execute()
{
    // TODO: Handle different lock/unlock patterns,
    // right now it just updates the texture on unlock
    auto *internalRaster = GetBackendRasterExt( m_pRaster );

    int64_t img_id = -1;

    DeviceGlobals::SharedMemoryTaskQueue->ExecuteTask(
        SharedMemoryTaskType::TEXTURE_LOAD,
        [this]( MemoryWriter &&writer ) {
            const uint32_t numMipLevels = 1;
            const bool     has_alpha    = true;

            RasterHeader header{
                .mWidth  = static_cast<uint32_t>( m_pRaster->width ),
                .mHeight = static_cast<uint32_t>( m_pRaster->height ),
                .mDepth  = 1,
                .mFormat = static_cast<uint32_t>(
                    rh::engine::ImageBufferFormat::BGRA8 ),
                .mMipLevelCount = numMipLevels };
            // serialize
            writer.Write( &header );

            for ( uint32_t i = 0; i < numMipLevels; i++ )
            {
                auto &mipLevelHeader = writer.Current<MipLevelHeader>();
                mipLevelHeader.mSize = m_pRaster->width * m_pRaster->height * 4;
                mipLevelHeader.mStride = m_pRaster->stride;
                writer.Skip( sizeof( MipLevelHeader ) );
                writer.Write( m_pRaster->cpPixels );
                writer.Skip( mipLevelHeader.mSize );
            }
        },
        [&img_id]( MemoryReader &&memory_reader ) {
            // deserialize
            img_id = *memory_reader.Read<int64_t>();
        } );
    internalRaster->mImageId = img_id < 0 ? 0xBADF00D : img_id;

    /* Restore the original width, height & stride */
    m_pRaster->width  = m_pRaster->originalWidth;
    m_pRaster->height = m_pRaster->originalHeight;

    m_pRaster->stride = 0;

    free( m_pRaster->cpPixels );
    m_pRaster->cpPixels = nullptr;
    return true;
}
