#include "rasterlockcmd.h"
#include <common_headers.h>
#include <rw_engine/rh_backend/raster_backend.h>
#include <rw_engine/system_funcs/raster_lock_cmd.h>

namespace rh::rw::engine
{
RwRasterLockCmd::RwRasterLockCmd( RwRaster *raster, int32_t accessMode )
    : mRaster( raster ), mAccessMode( accessMode )
{
}

bool RwRasterLockCmd::Execute( void *&res_data_ptr )
{
    if ( mRaster == nullptr )
        return false;
    /* Prepare lock info */
    auto mip_level = static_cast<uint8_t>( ( mAccessMode & 0xFF00 ) >> 8 );

    /* Pixels */
    mRaster->cpPixels = nullptr; //(RwUInt8 *)rasExt->lockedRect.pBits;

    /* Cache original width, height & stride */
    mRaster->originalWidth  = mRaster->width;
    mRaster->originalHeight = mRaster->height;

    /* Mip level width, height & stride */
    mRaster->width  = mRaster->width >> mip_level;
    mRaster->height = mRaster->height >> mip_level;

    /* Clamp width and height to 1 */
    if ( mRaster->width == 0 )
        mRaster->width = 1;
    if ( mRaster->height == 0 )
        mRaster->height = 1;

    auto backend_raster = GetBackendRasterExt( mRaster );

    if ( ( ( mAccessMode & rwRASTERLOCKREAD ) != 0 ) )
    {
        RasterLockCmdImpl lock_cmd{};
        auto              lock_result = lock_cmd.Invoke(
            { .mImageId = backend_raster->mImageId,
              .mWidth   = static_cast<uint32_t>( mRaster->width ),
              .mHeight  = static_cast<uint32_t>( mRaster->height ),
              .mLockMode = RasterLockParams::LockRead,
              .mMipLevel = mip_level } );

        mRaster->cpPixels = static_cast<uint8_t *>( malloc( static_cast<size_t>(
            lock_result.mLockDataHeight * lock_result.mLockDataStride ) ) );

        if ( lock_result.mData )
            std::memcpy( mRaster->cpPixels, lock_result.mData,
                         lock_result.mLockDataHeight *
                             lock_result.mLockDataStride );
        else
            debug::DebugLogger::Error(
                "Failed to lock texture, lock operation returned no data!" );

        /* Set the stride */
        mRaster->stride = lock_result.mLockDataStride;
    }
    else
    {
        constexpr auto pixel_size = 4;
        /* Set the stride */
        mRaster->stride = pixel_size * mRaster->width;

        mRaster->cpPixels = static_cast<uint8_t *>( malloc(
            static_cast<size_t>( mRaster->height * mRaster->stride ) ) );
    }
    if ( ( ( mAccessMode & rwRASTERLOCKWRITE ) != 0 ) )
        mRaster->privateFlags = 0x4;
    res_data_ptr = mRaster->cpPixels;

    return true;
}

} // namespace rh::rw::engine